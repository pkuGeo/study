//
//  main.cpp
//  ip_merge
//
//  Created by 周海洋 on 15/9/19.
//  Copyright © 2015年 周海洋. All rights reserved.

#include <iostream>
#include <fstream>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

unsigned log2(unsigned value)   //非递归判断一个数是2的多少次方
{
  unsigned x=0;
  while(value>1) {
    value>>=1;
    ++x;
  }
  return x;
}

int main(int argc, const char * argv[]) {
  std::string line;
  std::ifstream ifs("/Users/geo/ipcn.txt");
  if (!ifs.is_open()) {
    std::cerr<<"failed to open file"<<std::endl;
    return -1;
  }
  std::vector<std::pair<unsigned int, unsigned int> > ip_ranges;
  while (true) {
    std::getline(ifs, line);
    if (ifs.eof()) {
      break;
    }
    std::vector<std::string> ip_bits;
    boost::split(ip_bits, line, boost::is_any_of("./"));
    if (ip_bits.size()!=5) {
      std::cerr<<"error"<<std::endl;
      return -1;
    }
    unsigned int ip_start =
    (boost::lexical_cast<unsigned int>(ip_bits[0])<<24) +
    (boost::lexical_cast<unsigned int>(ip_bits[1])<<16) +
    (boost::lexical_cast<unsigned int>(ip_bits[2])<<8) +
    (boost::lexical_cast<unsigned int>(ip_bits[3])<<0);
    unsigned ip_end = ip_start + (1<<(32-boost::lexical_cast<unsigned int>(ip_bits[4]))) - 1;
    ip_ranges.push_back(std::make_pair(ip_start, ip_end));
  }
  
  std::vector<std::pair<unsigned int, unsigned int> > merged_ip_ranges;
  
  int merge_index = 1;
  std::pair<unsigned,unsigned> curr_ip_range = ip_ranges[0];
  
  while (merge_index < ip_ranges.size()) {
    std::pair<unsigned, unsigned>& to_be_merged = ip_ranges[merge_index];
    if (curr_ip_range.second + 1 == to_be_merged.first) {
      curr_ip_range.second = to_be_merged.second;
      ++merge_index;
      continue;
    }
    merged_ip_ranges.push_back(curr_ip_range);
    curr_ip_range = to_be_merged;
    ++merge_index;
  }
  merged_ip_ranges.push_back(curr_ip_range);
  
  std::vector<std::pair<unsigned int, unsigned int> > result_ip_ranges;
  
  for (size_t i = 0; i < merged_ip_ranges.size(); ++i) {
    unsigned ip_start = merged_ip_ranges[i].first;
    unsigned ip_end = merged_ip_ranges[i].second;
    unsigned size = ip_end - ip_start + 1;
    do {
      unsigned bit = log2(size);
      while (((ip_start + (1<<bit) - 1)&(0xFFFFFFFFU - ((1<<bit)-1)))!=ip_start) --bit;
      result_ip_ranges.push_back(std::make_pair(ip_start, 32-bit));
      ip_start += (1<<bit);
      size = ip_end - ip_start + 1;
    } while (size != 0);
  }
  for (size_t i = 0; i < result_ip_ranges.size(); ++i) {
    unsigned ip = result_ip_ranges[i].first;
    std::cout<<(ip>>24)<<"."<<((ip>>16)&0xFF)<<"."<<((ip>>8)&0xFF)<<"."<<(ip&0xFF)<<"/"<<result_ip_ranges[i].second<<std::endl;
  }
//  std::cout<<"original: "<<ip_ranges.size()<<std::endl;
//  std::cout<<"new size: "<<result_ip_ranges.size()<<std::endl;
  return 0;
}
