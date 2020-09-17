#ifndef CLASDIGITAL_SRC_SERVER_CACHE_CACHE_H
#define CLASDIGITAL_SRC_SERVER_CACHE_CACHE_H
#include <atomic>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <filesystem>
#include <shared_mutex>
#include <fstream>
#include <iostream>
#include <algorithm>


namespace clas_digital
{
  struct CacheableObject
  {
    using success_func_t = std::function<void(void*,unsigned long long)>;
    using error_func_t = std::function<void(int)>;

    virtual long long Load(const success_func_t &onsuccess, const error_func_t &onerror) = 0;
    virtual long long Unload() = 0;

    bool is_loaded_;
  };

  template<typename Key>
  class FixedSizeCache
  {
    public:
      using map_t = std::unordered_map<Key,std::shared_ptr<CacheableObject>>;
      using iterator_map_t = typename map_t::iterator;

      FixedSizeCache(long long maxSize) : size_(maxSize), free_space_(maxSize), lru_order_index_(0) {}

      bool Register(Key key, std::shared_ptr<CacheableObject> obj)
      {
        std::unique_lock lck(access_);
        auto ret = cache_objs_.insert({key,obj});
        if(!ret.second)
          return false;
        lru_order_.push_back(ret.first);
        if(free_space_>0)
          free_space_-=obj->Load([](void *,long long){},[](int err){});
        return true;
      }

      
      void Unregister(Key key)
      {
        std::unique_lock lck(access_);
        auto it = cache_objs_.find(key);
        if(it!=cache_objs_.end())
        {
          lru_order_.erase(std::remove(lru_order_.begin(), lru_order_.end(), it), lru_order_.end());
          cache_objs_.erase(it);
        }
      }

      void DebugPrint()
      {
        std::unique_lock lck(access_);
        for(const auto &it : cache_objs_)
        {
          std::cout<<"Registered: "<<it.first<<std::endl;
        }
        std::cout<<"Maximal size: "<<size_<<std::endl;
        std::cout<<"Current free space: "<<free_space_<<std::endl;
        std::cout<<"Vector size: "<<lru_order_.size()<<std::endl;
        std::cout<<"Vector pointer: "<<lru_order_index_<<std::endl;
      }

      void Load(Key key, CacheableObject::success_func_t onsuccess, CacheableObject::error_func_t onerror)
      {
        std::shared_ptr<CacheableObject> obj;
        try
        {
          std::shared_lock lck(access_);
          obj = cache_objs_.at(key);
          
          while(free_space_.load() < 0)
          {
            auto unload_lru = lru_order_[lru_order_index_.fetch_add(1)%lru_order_.size()];
            free_space_ += unload_lru->second->Unload();
          }
        }
        catch(...)
        {
          onerror(2);
          return;
        }
        free_space_ -= obj->Load(onsuccess,onerror);
      }

      auto size()
      {
        return cache_objs_.size();
      }

      auto free_space()
      {
        return free_space_.load();
      }

      auto total_space()
      {
        return size_;
      }

    private:
      const long long size_;
      std::atomic<long long> free_space_;
      
      std::atomic<long> lru_order_index_;
      std::vector<iterator_map_t> lru_order_;

      map_t cache_objs_;

      std::shared_mutex access_;
  };

  class UnmutableCacheableFile : public CacheableObject
  {
    public:
      UnmutableCacheableFile(std::filesystem::path path) : file_path_(path) 
      {
      }

      long long Load(const success_func_t &onsuccess, const error_func_t &onfailure) override
      {
        std::shared_ptr<std::vector<char>> local_data;
        
        {
          std::shared_lock lck(lock_);
          local_data = data_;
        }
        
        long long ret = 0;
        if(!local_data)
        {
          std::ifstream ifs(file_path_,std::ios::in|std::ios::binary);
          std::error_code ec;
          auto size = std::filesystem::file_size(file_path_,ec);
          if(!ifs.is_open())
          {
            onfailure(1);
            return 0;
          }
          local_data.reset(new std::vector<char>(size));
          ifs.read(local_data->data(),local_data->size());
          ifs.close();

          {
            std::unique_lock lck(lock_);
            if(!data_)
            {
              data_ = local_data;
              ret = local_data->size();
            }
          }
        }

        onsuccess(local_data->data(),local_data->size());
        return ret;
      }

      long long Unload() override
      {
        std::unique_lock lck(lock_);
        if(!data_)
          return 0;
        auto size = data_->size();
        data_.reset();
        return size;
      }

    private:
      std::filesystem::path file_path_;
      std::shared_mutex lock_;
      std::shared_ptr<std::vector<char>> data_;
  };


};


#endif
