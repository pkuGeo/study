//
//  main.cpp
//  pngconvert
//
//  Created by Haiyang Zhou on 13-12-17.
//  Copyright (c) 2013年 Haiyang Zhou. All rights reserved.
//

#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <queue>
#include "CPngConverter.h"

typedef struct {
    boost::mutex *m_mutex;
    std::queue<CPngConverter> *m_pTaskQueue;
    bool *m_pTaskEnd;
    bool m_convertEnd;
    int *m_converted;
} CONVERT_PARAM;

void convertThreadFunc(CONVERT_PARAM *param)
{
    while (true) {
        CPngConverter converter;
        param->m_mutex->lock();
        if (param->m_pTaskQueue->empty()) {
            if (*param->m_pTaskEnd) {
                param->m_mutex->unlock();
                break;
            }
            param->m_mutex->unlock();
            boost::this_thread::sleep(boost::posix_time::milliseconds(200));
            continue;
        }
        converter = param->m_pTaskQueue->front();
        param->m_pTaskQueue->pop();
        param->m_mutex->unlock();
        //CPngConverter::CONVERT_STATUS status = converter.convertToWebp();
        CPngConverter::CONVERT_STATUS status = converter.convert();
        if (status == CPngConverter::CONVERT_STATUS_OK || status == CPngConverter::CONVERT_STATUS_NONEED) {
            param->m_mutex->lock();
            ++*param->m_converted;
            param->m_mutex->unlock();
        } else{
            std::cerr<<"ERROR OCCURED!"<<std::endl;
            break;
        }
    }
    param->m_convertEnd = true;
}

void inputTask(const std::string& baseDir, const std::string &backupDir, int threadNum)
{
    assert(threadNum > 0);
    size_t queueCapacity = 2000;
    bool taskEnd = false;
    std::queue<CPngConverter> taskQueue;
    boost::mutex queueMutex;
    int converted = 0;
    int totoalFile = 0;
    int totalDir = 0;
    boost::filesystem::path backupPath(backupDir);
    if (!boost::filesystem::exists(backupPath)) {
        boost::filesystem::create_directories(backupDir);
        if (!boost::filesystem::exists(backupPath)
            || !boost::filesystem::is_directory(backupPath)) {
            std::cerr<<"Cannot Create Dir: "<<backupPath<<std::endl;
            return;
        }
    }
    
    std::queue<std::string> travelQueue;
    boost::filesystem::path basePath(baseDir);
    if (!boost::filesystem::exists(basePath)) {
		std::cerr<<"Cannot Open Path: "<<baseDir<<std::endl;
        return;
    }
    
    CONVERT_PARAM *params = new CONVERT_PARAM[threadNum];
    for (int i = 0; i < threadNum; ++i) {
        params[i].m_convertEnd = false;
        params[i].m_mutex = &queueMutex;
        params[i].m_pTaskEnd = &taskEnd;
        params[i].m_pTaskQueue = &taskQueue;
        params[i].m_converted = &converted;
        boost::thread convertThread(boost::bind(convertThreadFunc,params+i));
    }
    
    boost::filesystem::directory_iterator endIter;
    ++totalDir;
    for (boost::filesystem::directory_iterator iter(baseDir);
         iter != endIter;
         ++iter) {
        std::string leafName = iter->path().leaf().string();
        if (boost::filesystem::is_directory(*iter)) {
            travelQueue.push(leafName);
            continue;
        }
        while (taskQueue.size() >= queueCapacity) {
            boost::this_thread::sleep(boost::posix_time::milliseconds(200));
        }
        queueMutex.lock();
        taskQueue.push(CPngConverter(baseDir,backupDir,leafName));
        queueMutex.unlock();
        ++totoalFile;
    }
    
    while (!travelQueue.empty()) {
        std::string prefix = travelQueue.front();
        boost::filesystem::create_directory(backupDir+prefix);
        travelQueue.pop();
        ++totalDir;
        for (boost::filesystem::directory_iterator iter(baseDir+prefix);
             iter != endIter;
             ++iter) {
            std::string leafName = prefix+"/"+iter->path().leaf().string();
            if (boost::filesystem::is_directory(*iter)) {
                travelQueue.push(leafName);
                continue;
            }
            while (taskQueue.size() >= queueCapacity) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(200));
            }
            queueMutex.lock();
            taskQueue.push(CPngConverter(baseDir,backupDir,leafName));
            queueMutex.unlock();
            ++totoalFile;
        }
    }
    taskEnd = true;
    
    while (true) {
        bool finished = true;
        for (int i = 0; i < threadNum; ++i) {
            if (!params[i].m_convertEnd) {
                finished = false;
                break;
            }
        }
        if (finished) {
            break;
        } else {
            //睡觉，等其他线程执行完毕
            boost::this_thread::sleep(boost::posix_time::milliseconds(200));
        }
    }
    delete [] params;
    std::cout<<"Convert Succeeded!!"<<std::endl;
    std::cout<<"           Dirs: "<<totalDir<<std::endl;
    std::cout<<"          Files: "<<totoalFile<<std::endl;
    std::cout<<"Converted Files: "<<converted<<std::endl;
}

int main(int argc, const char * argv[])
{
    std::string baseDir = "/Users/Geo/Desktop/tmp";
    std::string backupDir = "/Users/Geo/Desktop/backup";
    int threadNum = 4;
//    if (argc < 2 || argc > 3) {
//        std::cerr<<"Usage:"<<std::endl;
//        std::cerr<<argv[0]<<" [base path], with default thread number 4"<<std::endl;
//        std::cerr<<"Or"<<std::endl;
//        std::cerr<<argv[0]<<" [base path] [thread number]"<<std::endl;
//        return -1;
//    }
    
//    baseDir = argv[1];
//    backupDir = argv[1];
    if (argc == 3) {
        threadNum = atoi(argv[2]);
        if (threadNum <= 0) {
            std::cerr<<"Wrong Thread Number! Must Be Positive Integer!"<<std::endl;
            return -1;
        }
    }
    if (*baseDir.rbegin()!='/'
        && *baseDir.rbegin()!='\\') {
        baseDir.append("/");
    }
    if (*backupDir.rbegin()!='/'
        && *backupDir.rbegin()!='\\') {
        backupDir.append("/");
    }
    inputTask(baseDir, backupDir, threadNum);
    return 0;
}

