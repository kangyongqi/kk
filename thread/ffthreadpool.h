#ifndef FFThreadPool_H
#define FFThreadPool_H

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include<atomic>

class FFThreadPool {
public:
    explicit FFThreadPool();
    ~FFThreadPool();

    void init(size_t threadCount_);
    void stop();
    void wait();

    template<typename Func>
    void submit(Func func){
        // 把各种形状的快递，都装进统一规格的盒子里.万能收纳盒!!!!!!!!!!
        std::function<void()> task = func;
        //enqueueTask(func);
        enqueueTask(task);
    }
    /*
    template<typename Func>
    void submit(Func func){
        enqueueTask(func);
    }
     */

//    template<typename Class,typename Func>
//    void submit(Class* obj,Func memberFunc){
//        std::function<void()> task = [obj,memberFunc](){
//            (obj->*memberFunc)();
//            delete obj;
//        };
//        enqueueTask(task);
//    }

//    template<typename Class,typename Func,typename... Args>
//    // obj:任务对象  memberFunc:成员函数指针  args:可变参数列表
//    void submit(Class*obj,Func memberFunc, Args&&... args){
//        std::function<void()> task =  [obj, memberFunc, args...]() mutable {
//            (obj->*memberFunc)(std::forward<Args>(args)...);
//            delete obj;
//        };
//        enqueueTask(task);
//    }

private:
    void work();
    std::function<void()> getTask();
    // 入队
    void enqueueTask(std::function<void()>task);

private:
    std::vector<std::thread> threadVec;
    std::queue<std::function<void()>> taskQueue;
    std::mutex mutex;
    std::condition_variable cond;
    bool m_stop;
    size_t threadCount;

};
#endif // FFFFThreadPool_H
