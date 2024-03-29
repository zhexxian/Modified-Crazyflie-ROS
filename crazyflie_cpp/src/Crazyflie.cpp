//#include <regex>
#include <mutex>
#include <stdint.h>

//#include "ros/ros.h"

#include "Crazyflie.h"
#include "crtp.h"

#include "Crazyradio.h"
#include "Crazywifi.h"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <thread>

#define MAX_RADIOS 16

Crazyradio* g_crazyradios[MAX_RADIOS];
std::mutex g_mutex[MAX_RADIOS];

Crazyflie::Crazyflie(
  const std::string& link_uri)
  : m_wifi(NULL)
  , m_ip_0(0)
  , m_ip_1(0)
  , m_ip_2(0)
  , m_ip_3(0)
  , m_port(0)
  , m_logInfo()
  , m_logTocEntries()
  , m_logTocEntriesRequested()
  , m_logBlockCb()
  , m_blockReset(false)
  , m_blockCreated()
  , m_blockStarted()
  , m_blockStopped()
  , m_paramInfo()
  , m_paramTocEntries()
  , m_paramTocEntriesRequested()
  , m_paramValues()
  , m_paramValuesRequested()
  , m_emptyAckCallback(nullptr)
  , m_linkQualityCallback(nullptr)
{

  bool success = false;
  // const char double_slash[2] = "//";
  // char* ip;
  // ip = strtok(link_uri.c_str(),double_slash);
  // const char single_slash[1] = "/";
  // ip = strtok(NULL,single_slash);
  // char* port;
  // port = 

  success = std::sscanf(link_uri.c_str(), "wifi://%d.%d.%d.%d/%d",
      &m_ip_0, &m_ip_1, &m_ip_2, &m_ip_3, &m_port) == 5;
   if (!success) {
      success = std::sscanf(link_uri.c_str(), "wifi://%d.%d.%d.%d",
           &m_ip_0, &m_ip_1, &m_ip_2, &m_ip_3) == 4;
      m_port = 23;
  }

  // success = std::sscanf(link_uri.c_str(), "wifi://%d.%d.%d.%d/%d",
  //     &m_ip_0, &m_ip_1, &m_ip_2, &m_ip_3, &m_port) == 5;
  //  if (!success) {
  //     success = std::sscanf(link_uri.c_str(), "wifi://%d.%d.%d.%d",
  //          &m_ip_0, &m_ip_1, &m_ip_2, &m_ip_3) == 4;
  //     m_port = 23;
  // }

  //throw std::runtime_error("IP: "+ std::string(ip)+ " port: " + std::string(port));
  if (success)
  {
      //printf("SHANYOOOOOOOOOOOO");
      //throw std::runtime_error("SHANYOOOOOOOOOOOO");
      char* ip;
      char* port;

      //sprintf(ip, "%d.%d.%d.%d", m_ip_0, m_ip_1, m_ip_2, m_ip_3);
      //sprintf(port, "%d", m_port);
      
      
      m_wifi = new Crazywifi(ip, port);

  }

  else {
      //throw std::runtime_error("IP address is not valid!");
      printf("IP address is not valid!");
  }
}

void Crazyflie::logReset()
{
  m_blockReset = false;

  do {
    crtpLogResetRequest request;
    while (!sendPacket((const uint8_t*)&request, sizeof(request))){
    	sendPacket((const uint8_t*)&request, sizeof(request));
    	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  } while (!m_blockReset);

  
}

void Crazyflie::sendSetpoint(
  float roll,
  float pitch,
  float yawrate,
  uint16_t thrust)
{
  crtpSetpointRequest request(roll, pitch, yawrate, thrust);
  //std::cout << "!!!!!!!!SENDING SET POINT" << std::endl;
  sendPacket((const uint8_t*)&request, sizeof(request));
}

void Crazyflie::sendExternalPositionUpdate(
  float x,
  float y,
  float z)
{
  crtpExternalPositionUpdate position(x, y, z);
  sendPacket((const uint8_t*)&position, sizeof(position));
}

void Crazyflie::sendPing()
{
  uint8_t ping = 0xFF;
  sendPacket(&ping, sizeof(ping));
}

// https://forum.bitcraze.io/viewtopic.php?f=9&t=1488
void Crazyflie::reboot()
{
  const uint8_t reboot_init[] = {0xFF, 0xFE, 0xFF};
  while(!sendPacket(reboot_init, sizeof(reboot_init))) {}

  const uint8_t reboot_to_firmware[] = {0xFF, 0xFE, 0xF0, 0x01};
  while(!sendPacket(reboot_to_firmware, sizeof(reboot_to_firmware))) {}
}

void Crazyflie::rebootToBootloader()
{
  const uint8_t reboot_init[] = {0xFF, 0xFE, 0xFF};
  while(!sendPacket(reboot_init, sizeof(reboot_init))) {}

  const uint8_t reboot_to_bootloader[] = {0xFF, 0xFE, 0xF0, 0x00};
  while(!sendPacket(reboot_to_bootloader, sizeof(reboot_to_bootloader))) {}
}

void Crazyflie::requestLogToc()
{

  // Find the number of log variables in TOC
  m_logInfo.len = 0;
  do
  {
    crtpLogGetInfoRequest request;
    sendPacket((const uint8_t*)&request, sizeof(request));
  } while(m_logInfo.len == 0);
  std::cout << "Log: " << (int)m_logInfo.len << std::endl;

  // Prepare data structures to request detailed information
  // m_logTocEntriesRequested.clear();
  // m_logTocEntries.resize(m_logInfo.len);
  // for (size_t i = 0; i < m_logInfo.len; ++i)
  // {
  //   m_logTocEntriesRequested.insert(i);
  // }

  // Request detailed information, until done
  // while (m_logTocEntriesRequested.size() > 0)
  // {
  //   for (size_t p = 0; p < m_logTocEntriesRequested.size(); ++p)
  //   {
  //     auto iter = m_logTocEntriesRequested.begin();
  //     for (size_t j = 0; j < p; ++j) {
  //       ++iter;
  //     }
  //     size_t i = *iter;
  //     crtpLogGetItemRequest request(i);
  //     sendPacket((const uint8_t*)&request, sizeof(request));
  //   }
  // }
}

void Crazyflie::requestParamToc()
{
   m_paramInfo.len = 0;

  // Find the number of log variables in TOC

  do
  {
    crtpParamTocGetInfoRequest request;
    sendPacket((const uint8_t*)&request, sizeof(request));
  } while(m_paramInfo.len == 0);




  // while(m_paramInfo.len == 0) {
  //   crtpParamTocGetInfoRequest request;
  //   while(!sendPacket((const uint8_t*)&request, sizeof(request))){
  //   	sendPacket((const uint8_t*)&request, sizeof(request));
  //   	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  //   }
  // }
    
  // do
  // {
  // 	std::cout << "#############doing " << std::endl;
  //   crtpParamTocGetInfoRequest request;
  //   sendPacket((const uint8_t*)&request, sizeof(request));
  //   if(m_paramInfo.len!=0){
  //   	std::cout << "#############BREAKING: " << std::endl;
  //   	break;
  //   }
  //   std::cout << "#############BEFORE SLEEPING: " << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  //   std::cout << "#############AFTER SLEEPING: " << std::endl;
  // } while(m_paramInfo.len == 0);
  std::cout << "Params: " << (int)m_paramInfo.len << std::endl;












  
 

  // while(m_paramInfo.len == 0) {
  // 	std::cout << "#############doing " << std::endl;
  //   crtpParamTocGetInfoRequest request;
  //   sendPacket((const uint8_t*)&request, sizeof(request));
  //   if(m_paramInfo.len!=0){
  //   	std::cout << "#############BREAKING: " << std::endl;
  //   	break;
  //   }
  //   std::cout << "#############BEFORE SLEEPING: " << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  //   std::cout << "#############AFTER SLEEPING: " << std::endl;
  // }
    
  // do
  // {
  // 	std::cout << "#############doing " << std::endl;
  //   crtpParamTocGetInfoRequest request;
  //   sendPacket((const uint8_t*)&request, sizeof(request));
  //   if(m_paramInfo.len!=0){
  //   	std::cout << "#############BREAKING: " << std::endl;
  //   	break;
  //   }
  //   std::cout << "#############BEFORE SLEEPING: " << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  //   std::cout << "#############AFTER SLEEPING: " << std::endl;
  // } while(m_paramInfo.len == 0);
  
  //std::cout << "Params: " << (int)m_paramInfo.len << std::endl;




  // Prepare data structures to request detailed information

  m_paramTocEntriesRequested.clear();
  m_paramValues.clear();
  m_paramTocEntries.resize(m_paramInfo.len);
  for (size_t i = 0; i < m_paramInfo.len; ++i)
  {
    m_paramTocEntriesRequested.insert(i);
    m_paramValuesRequested.insert(i);
  }


  // Request detailed information, until done

  while (m_paramTocEntriesRequested.size() > 0)
  {
    for (size_t p = 0; p < m_paramTocEntriesRequested.size(); ++p)
    {
      auto iter = m_paramTocEntriesRequested.begin();
      for (size_t j = 0; j < p; ++j) {
        ++iter;
      }
      size_t i = *iter;

      	crtpParamTocGetItemRequest request(i);
      	sendPacket((const uint8_t*)&request, sizeof(request));
    }
  }


  // Request current values

  while (m_paramValuesRequested.size() > 0)
  {
    for (size_t p = 0; p < m_paramValuesRequested.size(); ++p)
    {
      auto iter = m_paramValuesRequested.begin();
      for (size_t j = 0; j < p; ++j) {
        ++iter;
      }
      size_t i = *iter;
      crtpParamReadRequest request(i);
      sendPacket((const uint8_t*)&request, sizeof(request));
    }
  }


}

void Crazyflie::setParam(uint8_t id, const ParamValue& value) {

  m_paramValues.erase(id);
  for (auto&& entry : m_paramTocEntries) {
    if (entry.id == id) {
      do
      {
        switch (entry.type) {
          case ParamTypeUint8:
            {
              crtpParamWriteRequest<uint8_t> request(id, value.valueUint8);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
          case ParamTypeInt8:
            {
              crtpParamWriteRequest<int8_t> request(id, value.valueInt8);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
          case ParamTypeUint16:
            {
              crtpParamWriteRequest<uint16_t> request(id, value.valueUint16);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
          case ParamTypeInt16:
            {
              crtpParamWriteRequest<int16_t> request(id, value.valueInt16);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
          case ParamTypeUint32:
            {
              crtpParamWriteRequest<uint32_t> request(id, value.valueUint32);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
          case ParamTypeInt32:
            {
              crtpParamWriteRequest<int32_t> request(id, value.valueInt32);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
          case ParamTypeFloat:
            {
              crtpParamWriteRequest<float> request(id, value.valueFloat);
              sendPacket((const uint8_t*)&request, sizeof(request));
              break;
            }
        }
      } while(m_paramValues.find(id) == m_paramValues.end());
      break;
    }
  }


}

bool Crazyflie::sendPacket(
  const uint8_t* data,
  uint32_t length)
{
  //static uint32_t numPackets = 0;
  //static uint32_t numAcks = 0;

  //numPackets++;

  Crazywifi::Ack wifiack_data;

  // if (m_wifi->getAddress() != m_ip_address) {
  //    m_wifi->setAddress(m_ip_address);
  //    m_wifi->setPort(m_port);
  // }


  //blocking

  m_wifi->sendPacket(data, length, wifiack_data);

  // while (!wifiack_data.ack) {
  // 	Crazywifi::Ack wifiack_data;
  // 	m_wifi->sendPacket(data, length, wifiack_data);
  // 	std::this_thread::sleep_for(std::chrono::milliseconds(200));
  // }

  if (wifiack_data.ack) {
  	handleAck(wifiack_data);
  	return true;
  }

  

/* 
  //wifiack_header.data[wifiack_header.size] = 0;
  //wifiack_data.data[wifiack_data.size] = 0;
  if (wifiack_data.ack) {
    handleAck(wifiack_data);
    //numAcks++;
    //printf("handling wifiack\n");
  }
  else {
  	//printf("NOT handling wifiack\n");
  }

*/


  // if (numPackets == 100) {
  //   if (m_linkQualityCallback) {
  //     // We just take the ratio of sent vs. acked packets here
  //     // for a sliding window of 100 packets
  //     float linkQuality = numAcks / (float)numPackets;
  //     m_linkQualityCallback(linkQuality);
  //   }
  //   numPackets = 0;
  //   numAcks = 0;
  // }
  // return ack.ack;

  return true;

  //return wifiack_data.ack;
}

void Crazyflie::handleAck(
  const Crazywifi::Ack& result)
{
  if (crtpConsoleResponse::match(result)) {
    if (result.size > 0) {
      crtpConsoleResponse* r = (crtpConsoleResponse*)result.data;
      std::cout << r->text << std::endl;
      // printf("********Handle Ack - Console\n");

      // crtp* header = (crtp*)result.data;
      // std::cout << "Port: " << (int)header->port  << " Link: " << (int)header->link << " Channel: " << (int)header->channel << " Len: " << (int)result.size << std::endl;
      // for (size_t i = 1; i < result.size; ++i) {
      //    std::cout << "    " << (int)result.data[i] << std::flush;
      // }
      // std::cout << "    " << std::endl;
    }
  }
  else if (crtpLogGetInfoResponse::match(result)) {
    crtpLogGetInfoResponse* r = (crtpLogGetInfoResponse*)result.data;
    m_logInfo.len = r->log_len;
    //printf("********Handle Ack - Log Get Info\n");
  }
  else if (crtpLogGetItemResponse::match(result)) {
    crtpLogGetItemResponse* r = (crtpLogGetItemResponse*)result.data;
    if (r->request.id < m_logTocEntries.size())
    {
      m_logTocEntriesRequested.erase(r->request.id);
      LogTocEntry& entry = m_logTocEntries[r->request.id];
      entry.id = r->request.id;
      entry.type = (LogType)r->type;
      entry.group = std::string(&r->text[0]);
      entry.name = std::string(&r->text[entry.group.size() + 1]);
    }
    //printf("********Handle Ack - Log Get Item Response\n");
    // std::cout << "Got: " << (int)r->id << std::endl;
  }
  else if (crtpLogControlResponse::match(result)) {
    crtpLogControlResponse* r = (crtpLogControlResponse*)result.data;
    if (r->command == 0 &&
        (r->result == crtpLogControlResultOk || r->result == crtpLogControlResultBlockExists)) {
      m_blockCreated.insert(r->requestByte1);
    }
    if (r->command == 3 && r->result == 0) {
      m_blockStarted.insert(r->requestByte1);
    }
    if (r->command == 4 && r->result == 0) {
      m_blockStopped.insert(r->requestByte1);
    }
    if (r->command == 5 && r->result == 0) {
      m_blockReset = true;
    }
    // std::cout << "LogControl: " << (int)r->command << " errno: " << (int)r->result << std::endl;
    // printf("********Handle Ack - Log Control Response\n");
    //  crtp* header = (crtp*)result.data;
    //   std::cout << "Port: " << (int)header->port  << " Link: " << (int)header->link << " Channel: " << (int)header->channel << " Len: " << (int)result.size << std::endl;
    //   for (size_t i = 1; i < result.size; ++i) {
    //      std::cout << "    " << (int)result.data[i] << std::flush;
    //   }
    //   std::cout << "    " << std::endl;
  }
  else if (crtpLogDataResponse::match(result)) {
    crtpLogDataResponse* r = (crtpLogDataResponse*)result.data;
    auto iter = m_logBlockCb.find(r->blockId);
    if (iter != m_logBlockCb.end()) {
      iter->second(r, result.size - 5);
    }
    else if (m_blockReset) {
      std::cout << "Received unrequested data for block: " << (int)r->blockId << std::endl;
    }
    //printf("********Handle Ack - Log Data Response\n");
  }
  else if (crtpParamTocGetInfoResponse::match(result)) {
    crtpParamTocGetInfoResponse* r = (crtpParamTocGetInfoResponse*)result.data;
    m_paramInfo.len = r->numParam;
    //std::cout << "r->numParam: " << (int)r->numParam << std::endl;
    // printf("********Handle Ack - ParamTocGetInfoResponse\n");
    //   crtp* header = (crtp*)result.data;
    //   std::cout << "Port: " << (int)header->port  << " Link: " << (int)header->link << " Channel: " << (int)header->channel << " Len: " << (int)result.size << std::endl;
    //   for (size_t i = 1; i < result.size; ++i) {
    //      std::cout << "    " << (int)result.data[i] << std::flush;
    //   }
    //   std::cout << "    " << std::endl;

	      // requestParamToc();

  }
  else if (crtpParamTocGetItemResponse::match(result)) {
    crtpParamTocGetItemResponse* r = (crtpParamTocGetItemResponse*)result.data;
    if (r->request.id < m_paramTocEntries.size())
    {
      m_paramTocEntriesRequested.erase(r->request.id);
      ParamTocEntry& entry = m_paramTocEntries[r->request.id];
      entry.id = r->request.id;
      entry.type = (ParamType)(r->length | r-> type << 2 | r->sign << 3);
      entry.readonly = r->readonly;
      entry.group = std::string(&r->text[0]);
      entry.name = std::string(&r->text[entry.group.size() + 1]);
    }
    //printf("********Handle Ack - ParamTocGetItemResponse\n");
  }
  else if (crtpParamValueResponse::match(result)) {
    crtpParamValueResponse* r = (crtpParamValueResponse*)result.data;
    ParamValue v;
    std::memcpy(&v, &r->valueFloat, 4);
    // *v = r->valueFloat;
    m_paramValues[r->id] = v;//(ParamValue)(r->valueFloat);
    m_paramValuesRequested.erase(r->id);
    //printf("********Handle Ack - ParamValueResponse\n");
  }
  else if (crtpPlatformRSSIAck::match(result)) {
    crtpPlatformRSSIAck* r = (crtpPlatformRSSIAck*)result.data;
    if (m_emptyAckCallback) {
      m_emptyAckCallback(r);
    }
    //printf("********Handle Ack - PlatformRSSIAck\n");
  }
  else {
    crtp* header = (crtp*)result.data;
    std::cout << "Don't know ack: Port: " << (int)header->port  << " Link: " << (int)header->link  << " Channel: " << (int)header->channel << " Len: " << (int)result.size << std::endl;
    for (size_t i = 1; i < result.size; ++i) {
       std::cout << "    " << (int)result.data[i] << std::flush;
    }
    std::cout << "    " << std::endl;
    //printf("********Handle Ack - Don't know Ack\n");
  }

}

const Crazyflie::LogTocEntry* Crazyflie::getLogTocEntry(
  const std::string& group,
  const std::string& name) const
{
  for (auto&& entry : m_logTocEntries) {
    if (entry.group == group && entry.name == name) {
      return &entry;
    }
  }
  return nullptr;
}

const Crazyflie::ParamTocEntry* Crazyflie::getParamTocEntry(
  const std::string& group,
  const std::string& name) const
{
  for (auto&& entry : m_paramTocEntries) {
    if (entry.group == group && entry.name == name) {
      return &entry;
    }
  }
  return nullptr;
}

uint8_t Crazyflie::registerLogBlock(
  std::function<void(crtpLogDataResponse*, uint8_t)> cb)
{
  for (uint8_t id = 0; id < 255; ++id) {
    if (m_logBlockCb.find(id) == m_logBlockCb.end()) {
      m_logBlockCb[id] = cb;
      return id;
    }
  }
}

bool Crazyflie::unregisterLogBlock(
  uint8_t id)
{
  m_logBlockCb.erase(m_logBlockCb.find(id));
}

