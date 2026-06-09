#ifndef FFGlobalClock_H
#define FFGlobalClock_H

#include<mutex>
#include<chrono>
#include<atomic>
class FFGlobalClock
{


public:
    FFGlobalClock();
    // 单例模式
    static FFGlobalClock *getInstance();
    // 获取当前音频时钟值
    int64_t getAudioClock();
    // 获取当前视频时钟值
    int64_t getVideoClock();
    // 设置音频时钟值
    void setAudioClock(int64_t clock);
    // 设置视频时钟值
    void setVideoClock(int64_t clock);
    // 初始化时钟
    void initClock();
    // 设置时钟就绪标志
    void setReadyFlag();
    // 获取时钟就绪标志
    bool getReadyFlag();
    // 设置全局基准时钟值
    void setClock(int64_t clock);
    // 获取全局基准时钟值
    int64_t getClock();

private:
    // 存储音频播放的当前时间戳，原子类型保证多线程读写的线程安全，无需额外加锁，效率更高
    std::atomic<int64_t> audioClock;
    std::atomic<int64_t> videoClock;// 原子操作的int64_t（存时间戳）
    // 标记时钟是否完成初始化，用于控制音视频同步逻辑的启动时机
    std::atomic<bool> readyFlag;
    std::mutex mutex;
    int64_t clock = 0;
};

#endif // FFGlobalClock_H
