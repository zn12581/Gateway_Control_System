#include "logger.h"
#include "rabbitmqclient.h"

using namespace utility;

/**
 * @description: 构造函数
 */
RabbitMqClient::RabbitMqClient(const std::string &hostname, int port, const std::string &user, const std::string &password) :  m_hostname(hostname),
                                                                                                                m_port(port),
                                                                                                                m_username(user),
                                                                                                                m_password(password),
                                                                                                                m_conn(nullptr),
                                                                                                                m_channel(1)
{
}

/**
 * @description: 析构函数
 */
RabbitMqClient::~RabbitMqClient()
{

}

/*************************************************************************
 *
 * Public Function
 *
 *************************************************************************/

/**
 * @description: 建立连接
 */
int RabbitMqClient::connect()
{
    /* 检测参数 */
    if (m_hostname.empty() || m_port <= 0 || m_username.empty() || m_password.empty())
    {
        log_error("RabbitMqClient建立连接时参数错误");
        return -1;
    }

    /* 1、分配新连接指针 */
    m_conn = amqp_new_connection();
    if (m_conn == nullptr)
    {
        log_error("RabbitMqClient分配连接器失败");
        return -2;
    }

    /* 2、创建socket */
    amqp_socket_t *m_socket = amqp_tcp_socket_new(m_conn);
    if (m_socket == nullptr)
    {
        log_error("RabbitMqClient分配网络端口失败");
        return -3;
    }

    /* 3、绑定主机地址和端口，建立与服务器连接 */
    int isOpenSocket = amqp_socket_open(m_socket, m_hostname.c_str(), m_port);
    if (isOpenSocket < 0)
    {
        log_error("RabbitMqClient绑定ip和端口失败");
        return -4;
    }

    /* 4、登录到RabbitMq服务器 */
    int isLogin = errorMsg(amqp_login(m_conn,
                                      "/",
                                      1,
                                      AMQP_DEFAULT_FRAME_SIZE,
                                      AMQP_DEFAULT_HEARTBEAT,
                                      AMQP_SASL_METHOD_PLAIN,
                                      m_username.c_str(),
                                      m_password.c_str()),
                           "建立连接");
    if (isLogin < 0)
    {
        log_error("RabbitMqClient登录服务器失败");
        return -5;
    }

    /* 6、打开通道 */
    amqp_channel_open_ok_t *isOpenChannel = amqp_channel_open(m_conn, m_channel);
    if (!isOpenChannel)
    {
        log_error("RabbitMqClient打开通道失败");
        return -6;
    }
    return 0;
}

/**
 * @description: 断开连接
 */
int RabbitMqClient::disconnect()
{
    int ret = 0;
    if (m_conn != nullptr)
    {
        /* 1、关闭通道 */
        ret = errorMsg(amqp_channel_close(m_conn, m_channel, AMQP_REPLY_SUCCESS), "关闭通道");
        if (ret < 0)
        {
            log_error("RabbitMqClient关闭通道失败");
            return ret;
        }

        /* 2、关闭连接 */
        ret = errorMsg(amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS), "关闭连接");
        if (ret < 0)
        {
            log_error("RabbitMqClient关闭连接失败");
            return ret;
        }

        /* 3、销毁连接 */
        ret = amqp_destroy_connection(m_conn);
        if (ret < 0)
        {
            log_error("RabbitMqClient注销连接器");
            return ret;
        }

        m_conn = nullptr;
    }
    else
    {
        log_warning("RabbitMqClient尝试打开一个不存在的连接");
        return -4;
    }

    return 0;
}

/**
 * @description: 初始化交换机
 */
int RabbitMqClient::declareExchange(CExchange &exchange)
{
    /* 声明交换机 */
    amqp_exchange_declare(m_conn,
                          m_channel,
                          amqp_cstring_bytes(exchange.m_name.c_str()),
                          amqp_cstring_bytes(exchange.m_type.c_str()),
                          exchange.m_passive,
                          exchange.m_durable,
                          exchange.m_autodelete,
                          exchange.m_internal,
                          amqp_empty_table);

    return errorMsg(amqp_get_rpc_reply(m_conn), "声明交换器");
}

/**
 * @description: 初始化队列
 */
int RabbitMqClient::declareQueue(CQueue &queue)
{
    /* 声明队列 */
    amqp_queue_declare(m_conn,
                       m_channel,
                       amqp_cstring_bytes(queue.m_name.c_str()),
                       queue.m_passive,
                       queue.m_durable,
                       queue.m_exclusive,
                       queue.m_autodelete,
                       amqp_empty_table);

    return errorMsg(amqp_get_rpc_reply(m_conn), "声明队列");
}

/**
 * @description: 将指定队列绑定到交换机上，在direct模式下bindkey可以为队列名称
 */
int RabbitMqClient::bindQueueToExchange(const std::string &queue, const std::string &exchange, const std::string &bindkey)
{
    amqp_queue_bind(m_conn,
                    m_channel,
                    amqp_cstring_bytes(queue.c_str()),
                    amqp_cstring_bytes(exchange.c_str()),
                    amqp_cstring_bytes(bindkey.c_str()),
                    amqp_empty_table);

    return errorMsg(amqp_get_rpc_reply(m_conn), "绑定队列到交换机");
}

/**
 * @description: 发布消息
 */
int RabbitMqClient::publish(const std::string &exchange_name, const std::string &routing_key_name, const CMessage &message)
{
    int ret = amqp_basic_publish(m_conn,
                                 m_channel,
                                 amqp_cstring_bytes(exchange_name.c_str()),
                                 amqp_cstring_bytes(routing_key_name.c_str()),
                                 message.m_mandatory,
                                 message.m_immediate,
                                 &message.m_properties,
                                 amqp_cstring_bytes(message.m_data.c_str()));
    if (ret != AMQP_STATUS_OK)
    {
        log_error("RabbitMq客户端发布消息出错");
        return errorMsg(amqp_get_rpc_reply(m_conn), "发布消息");
    }

    return 0;
}

/**
 * @description: 非阻塞方式消费，底层以amqp的get和read方法实现，每次主动向服务器拉取一条消息
 */
std::string RabbitMqClient::get(const std::string &queue_name, bool no_ack)
{
    std::vector<std::string> vec_msg = get(queue_name, 1, no_ack);
    if (vec_msg.size() != 1)
    {
        log_warning("尝试以非阻塞读取一条消息,但是获得了NULL或多个");
        return "";
    }

    return vec_msg[0];
}
std::vector<std::string> RabbitMqClient::get(const std::string &queue_name, int num, bool no_ack)
{
    std::vector<std::string> ret_msg;

    while (num--)
    {
        /*  1、阻塞同步轮询服务器中队列 */
        amqp_rpc_reply_t replyGet = amqp_basic_get(m_conn, m_channel, amqp_cstring_bytes(queue_name.c_str()), no_ack);
        int retGetMsg = errorMsg(replyGet, "Get message");
        if (retGetMsg < 0)
        {
            log_error("Failed to get message from RabbitMQ server");
            return ret_msg;
        }
        // 获取队列中存在多少条消息
        amqp_basic_get_ok_t *tip;
        switch(replyGet.reply.id)
        {
            case AMQP_BASIC_GET_OK_METHOD:
                tip = (amqp_basic_get_ok_t *)replyGet.reply.decoded;
                // info("rabbitmq queue remaining %d messages", tip->message_count);
                break;
            case AMQP_BASIC_GET_EMPTY_METHOD:
                log_info("no message in rabbitmq queue");
                return ret_msg;
            default:
                log_error("get error rabbitmq reply id %d", replyGet.reply.id);
                return ret_msg;
        }

        /* 2、读取chennal上的一条消息 */
        amqp_message_t amqp_msg;
        int retReadMsg = errorMsg(amqp_read_message(m_conn, m_channel, &amqp_msg, false), "Read message");
        if (retReadMsg < 0)
        {
            log_error("Failed to read rabbitmq message");
            return ret_msg;
        }

        /* 3、封装消息 */
        ret_msg.emplace_back(std::string((char *)amqp_msg.body.bytes, (char *)amqp_msg.body.bytes + amqp_msg.body.len));

        /* 4、应答ACK */
        if(no_ack == false)
        {
            amqp_basic_ack(m_conn, m_channel, tip->delivery_tag, false);
        }
    }

    return ret_msg;
}


/**
 * @description: 阻塞方式消费，底层为consume实现，本地被动一次性拉取服务器所有消息，依次由客户端消费
 */

void RabbitMqClient::consume_listen(const std::string &queue_name, struct timeval *timeout, bool no_ack)
{
    std::thread(
        [this, queue_name, timeout, no_ack] ()
        {
            try
            {
                // /* 1、设置通道消费的限制 */
                // amqp_basic_qos_ok_t *retQosOk = amqp_basic_qos(m_conn,
                //                                                m_channel,
                //                                                0,               // 预取消息的字节数prefetch_size 0：不限制大小
                //                                                prefetch_count,  // 预取消息的数量prefetch_count
                //                                                false);          // 是否将预取条件应用到整个通道 0：不应用
                // if (!retQosOk)
                // {
                //     errorMsg(amqp_get_rpc_reply(m_conn), "Set consumer limit(qos)");
                //     throw std::runtime_error("Basic qoe");
                // }

                /* 2、创建消费者 */
                amqp_basic_consume_ok_t *retBasicConsume = amqp_basic_consume(m_conn,
                                                                            m_channel,
                                                                            amqp_cstring_bytes(queue_name.c_str()),
                                                                            amqp_empty_bytes,
                                                                            false,  // no_local 0:接收 1:不接收
                                                                            no_ack, // no_ack 是否需要ack才将该消息从队列删除 0:需要调用amqp_basic_ack后才会清除 1:不回复
                                                                            false,  // exclusive 0:不独占 1:当前连接不在时队列自动删除
                                                                            amqp_empty_table);
                if (!retBasicConsume)
                {
                    errorMsg(amqp_get_rpc_reply(m_conn), "Consumer basic");
                    throw std::runtime_error("Consumer basic");
                }

                for(;;)
                {
                    // amqp_maybe_release_buffers(m_conn);

                    /* 3、消费 */
                    amqp_envelope_t envelope;
                    int isConsume = errorMsg(amqp_consume_message(m_conn, &envelope, timeout, 0), "Consume message");
                    if (isConsume < 0)
                    {
                        log_error("Faild to consume message from rabbitmq server");
                        throw std::runtime_error("Consume message");
                    }

                    /* 4、封装消息 */
                    r_store.enqueue(std::string((char *)envelope.message.body.bytes, (char *)envelope.message.body.bytes + envelope.message.body.len));

                    /* 5、应答ACK */
                    if(no_ack == false)
                    {
                        amqp_basic_ack(m_conn, m_channel, envelope.delivery_tag, false);
                    }

                    /* 6、删除封装容器 */
                    amqp_destroy_envelope(&envelope);
                }
            }
            catch (const std::exception &e)
            {
                log_error("RabbitMqClient消费异常, msg = {}", e.what());
            }
        }
    ).detach();
}

bool RabbitMqClient::consume(std::string &msg, bool block)
{
    return r_store.dequeue(msg, true); 
}

/*************************************************************************
 *
 * Private Function
 *
 *************************************************************************/

/**
 * @description: 处理错误信息
 */
int RabbitMqClient::errorMsg(const amqp_rpc_reply_t &reply, const std::string &desc)
{
    amqp_connection_close_t *de;
    switch (reply.reply_type)
    {
        case AMQP_RESPONSE_NORMAL:
            return 0;

        case AMQP_RESPONSE_NONE:
            log_error("RabbitMQ{}时,发生Response None错误", desc.c_str());
            break;

        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            log_error("RabbitMQ{}时,发生Response Library错误", desc.c_str());
            break;

        case AMQP_RESPONSE_SERVER_EXCEPTION:
            // switch(reply.reply.id)
            // {
            //     case AMQP_CONNECTION_CLOSE_METHOD:
            //         de = (amqp_connection_close_t *)reply.reply.decoded;
            //         log_error("RabbitMQ{}时,发生Response Server的CONNECTION CLOSE错误({}), msg = ", desc.c_str(), de->reply_code, std::string((char *)de->reply_text.bytes, de->reply_text.len));
            //         break;
            //     case AMQP_CHANNEL_CLOSE_METHOD:
            //         de = (amqp_connection_close_t *)reply.reply.decoded;
            //         log_error("RabbitMQ{}时,发生Response Server的CHANNEL CLOSE错误({}), msg = ", desc.c_str(), de->reply_code, std::string((char *)de->reply_text.bytes, de->reply_text.len));
            //         break;
            //     default:
            //         log_error("RabbitMQ{}时,发生未知的Response Server错误", desc.c_str());
            //         break;
            // }
            log_error("RabbitMQ{}时,发生未知的Response Server错误", desc.c_str());
            break;

        default:
            break;
    }

    return -1;
}

/**
 * @description: 获取RabbitMq版本
 */
void utility::showRabbitMqVersion()
{
    log_info("Rabbitmq Version %d.%d.%d", AMQP_VERSION_MAJOR, AMQP_VERSION_MINOR, AMQP_VERSION_PATCH);
}
