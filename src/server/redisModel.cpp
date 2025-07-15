#include "redisModel.h"
#include <iostream>

RedisModel::RedisModel(const std::string &host, int port, const std::string &password, int db)
    : m_running(false)
{
    try
    {
        Redis_ConnectionOptions opts;
        opts.host = host;
        opts.port = port;
        opts.db = db;
        opts.password = password;

        mp_redis = std::make_unique<Redis>(opts);
        mp_subscriber = std::make_unique<Redis_Subscriber>(mp_redis->subscriber());

        std::cout << "Redis连接成功: " << host << ":" << port << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Redis连接失败: " << e.what() << std::endl;
        throw;
    }
}

RedisModel::~RedisModel()
{
    m_stop();
}

bool RedisModel::m_publish(const std::string &channel, const std::string &message)
{
    try
    {
        long long subscribers = mp_redis->publish(channel, message);
        std::cout << "发布消息到频道 [" << channel << "]: " << message
                  << " (订阅者数量: " << subscribers << ")" << std::endl;
        return subscribers > 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "发布消息失败: " << e.what() << std::endl;
        return false;
    }
}

bool RedisModel::m_subscribe(const std::string &channel, MessageCallback &&callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        m_mapChannelCallbacks[channel] = std::move(callback);
        mp_subscriber->subscribe(channel);
        std::cout << "订阅频道: " << channel << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "订阅频道失败: " << e.what() << std::endl;
        m_mapChannelCallbacks.erase(channel);
        return false;
    }
}

bool RedisModel::m_psubscribe(const std::string &pattern, PatternMessageCallback &&callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        m_mapPatternCallbacks[pattern] = std::move(callback);
        mp_subscriber->psubscribe(pattern);
        std::cout << "模式订阅: " << pattern << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "模式订阅失败: " << e.what() << std::endl;
        m_mapPatternCallbacks.erase(pattern);
        return false;
    }
}

bool RedisModel::m_unsubscribe(const std::string &channel)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        mp_subscriber->unsubscribe(channel);
        m_mapChannelCallbacks.erase(channel);
        std::cout << "取消订阅频道: " << channel << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "取消订阅失败: " << e.what() << std::endl;
        return false;
    }
}

bool RedisModel::m_punsubscribe(const std::string &pattern)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        mp_subscriber->punsubscribe(pattern);
        m_mapPatternCallbacks.erase(pattern);
        std::cout << "取消模式订阅: " << pattern << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "取消模式订阅失败: " << e.what() << std::endl;
        return false;
    }
}

void RedisModel::m_start()
{
    if (m_running.load())
    {
        std::cout << "订阅服务已经在运行中" << std::endl;
        return;
    }

    m_running.store(true);
    m_subscribeThread = std::thread([this]()
                                    { this->m_messageLoop(); });

    std::cout << "开始监听Redis消息..." << std::endl;
}

void RedisModel::m_stop()
{
    if (!m_running.load())
    {
        return;
    }

    m_running.store(false);

    // 取消所有订阅以中断阻塞
    try
    {
        mp_subscriber->unsubscribe();
        mp_subscriber->punsubscribe();
    }
    catch (const std::exception &e)
    {
        std::cerr << "停止订阅时出错: " << e.what() << std::endl;
    }

    if (m_subscribeThread.joinable())
    {
        m_subscribeThread.join();
    }

    std::cout << "Redis订阅服务已停止" << std::endl;
}

size_t RedisModel::m_getChannelCount()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_mapChannelCallbacks.size();
}

size_t RedisModel::m_getPatternCount()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_mapPatternCallbacks.size();
}

void RedisModel::m_messageLoop()
{
    while (m_running.load())
    {
        try
        {
            mp_subscriber->consume();
        }
        catch (const sw::redis::TimeoutError &e)
        {
            // 超时是正常的，继续循环
            continue;
        }
        catch (const std::exception &e)
        {
            if (m_running.load())
            {
                std::cerr << "消息循环出错: " << e.what() << std::endl;
                break; // 如果不是因为停止而退出，则打印错误并退出循环
            }
        }
    }

    std::cout << "消息循环已退出" << std::endl;
}

void RedisModel::m_setupCallbacks()
{
    mp_subscriber->on_message([this](const std::string &channel, const std::string &msg)
                              {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_mapChannelCallbacks.find(channel);
            if (it != m_mapChannelCallbacks.end()) {
                try {
                    it->second(channel, msg);
                } catch (const std::exception& e) {
                    std::cerr << "处理消息回调时出错: " << e.what() << std::endl;
                }
            } });

    mp_subscriber->on_pmessage([this](const std::string &pattern,
                                      const std::string &channel,
                                      const std::string &msg)
                               {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_mapPatternCallbacks.find(pattern);
            if (it != m_mapPatternCallbacks.end()) {
                try {
                    it->second(pattern, channel, msg);
                } catch (const std::exception& e) {
                    std::cerr << "处理模式消息回调时出错: " << e.what() << std::endl;
                }
            } });
}

bool RedisModel::m_isRunning() const
{
    return m_running.load();
}