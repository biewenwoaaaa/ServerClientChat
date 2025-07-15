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
    explicit RedisModel(const std::string &host = "127.0.0.1",
                        int port = 6379,
                        const std::string &password = "",
                        int db = 0);

    ~RedisModel();

   
    RedisModel(const RedisModel &) = delete;
    RedisModel &operator=(const RedisModel &) = delete;

    bool m_publish(const std::string &channel, const std::string &message);

    // subscribe channel
    bool m_subscribe(const std::string &channel, MessageCallback &&callback);

    // subscribe pattern channel
    bool m_psubscribe(const std::string &pattern, PatternMessageCallback &&callback);

    // unsubscribe channel
    bool m_unsubscribe(const std::string &channel);

    // unsubscribe pattern channel
    bool m_punsubscribe(const std::string &pattern);

    // start listening for messages
    void m_start();

    // stop listening for messages
    void m_stop();

    // check if the subscriber is running
    bool m_isRunning() const;

    // get the number of subscribed channels
    size_t m_getChannelCount();

    // get the number of subscribed patterns
    size_t m_getPatternCount();

    // setup callbacks for channels and patterns
    void m_setupCallbacks();

private:
    // message loop for processing incoming messages
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
