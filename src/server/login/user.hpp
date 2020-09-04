#ifndef CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#define CLASDIGITAL_SRC_SERVER_LOGIN_USER_H
#include <sqlite3.h>
#include <SQLiteCpp/SQLiteCpp.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <filesystem>

enum class UserAccess
{
  READ = 1,
  WRITE = 2,
  ADMIN = 4
};

template <typename T>
class ThreadSafeQueue
{
 public:
 
  T pop()
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
      cond_.wait(mlock);
    }
    auto item = queue_.front();
    queue_.pop();
    return item;
  }
 
  void push(const T item)
  {
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push(item);
    mlock.unlock();
    cond_.notify_one();
  }
 
 
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};



class UserTable
{
  public:
    enum class ReturnCodes
    {
      OK = 1,
      USER_EXISTS = 2,
      UNKNOWN_ERROR = 3,
    };

    ReturnCodes Load(std::filesystem::path database_path);
    ReturnCodes AddUser(std::string email, std::string password, std::string name, UserAccess acc);
    ReturnCodes RemoveUser(std::string email);

    ~UserTable();
    UserTable();

  private:
    ThreadSafeQueue<SQLite::Database*> connections_;
    bool initialised_;
};

#endif
