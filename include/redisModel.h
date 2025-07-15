#pragma once

#include <sw/redis++/redis++.h>
#include <functional>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <memory>

class RedisModel
{
public:
    using MessageCallback = std::function<void(const std::string &channel, const std::string &message)>;
    using PatternMessageCallback = std::function<void(const std::string &pattern, const std::string &channel, const std::string &message)>;
    using Redis = sw::redis::Redis;
    using Redis_Subscriber = sw::redis::Subscriber;
    using Redis_ConnectionOptions = sw::redis::ConnectionOptions;

public:
    // 构造函数
    explicit RedisModel(const std::string &host = "127.0.0.1",
                        int port = 6379,
                        const std::string &password = "",
                        int db = 0);

    // 析构函数
    ~RedisModel();

    // 禁用拷贝
    RedisModel(const RedisModel &) = delete;
    RedisModel &operator=(const RedisModel &) = delete;

    // 发布消息
    bool m_publish(const std::string &channel, const std::string &message);

    // 订阅频道
    bool m_subscribe(const std::string &channel, MessageCallback &&callback);

    // 模式订阅
    bool m_psubscribe(const std::string &pattern, PatternMessageCallback &&callback);

    // 取消订阅频道
    bool m_unsubscribe(const std::string &channel);

    // 取消模式订阅
    bool m_punsubscribe(const std::string &pattern);

    // 开始监听消息
    void m_start();

    // 停止监听
    void m_stop();

    // 检查是否正在运行
    bool m_isRunning() const;

    // 获取订阅的频道数量
    size_t m_getChannelCount();

    // 获取模式订阅数量
    size_t m_getPatternCount();
    // 设置消息处理回调
    void m_setupCallbacks();

private:
    // 消息循环
    void m_messageLoop();

private:
    std::unique_ptr<Redis> mp_redis;
    std::unique_ptr<Redis_Subscriber> mp_subscriber;
    std::thread m_subscribeThread;
    std::atomic<bool> m_running;

    std::unordered_map<std::string, MessageCallback> m_mapChannelCallbacks;
    std::unordered_map<std::string, PatternMessageCallback> m_mapPatternCallbacks;
    std::mutex m_mutex;
};
