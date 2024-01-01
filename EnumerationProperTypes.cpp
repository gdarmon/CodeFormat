/*
 * EnumerationProperTypes.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: aharonh
 */

#include "functionality/calibration/EnumerationProperTypes.h"

OptimizeEnumeration::OptimizeEnumeration(int val,const EnumDictionary& dictionary)
:_val(val),
 _dictionary(dictionary){}

OptimizeEnumeration::OptimizeEnumeration(const OptimizeEnumeration& other)
:_dictionary(other._dictionary)
{
  *this = other;
}

OptimizeEnumeration::operator const int() const
{
  return _val;
}
OptimizeEnumeration::~OptimizeEnumeration(){}
MEtl::string OptimizeEnumeration::operator[](int val) const
{
  return _dictionary[val];
}

int OptimizeEnumeration::operator[](const MEtl::string& strVal) const
{
  return _dictionary[strVal];
}

const OptimizeEnumeration& OptimizeEnumeration::operator=(const OptimizeEnumeration& other)
{
  _val = other._val;
  return *this;
}


OptimizeEnumeration& atot(OptimizeEnumeration& t, const MEtl::string& a){
  t._val = t[a];
  if(t._val == EnumDictionary::INVALID_VAL){
    fprintf(stderr, "can't find matching enumeration for string value %s\n", a.c_str());
  }
  return t;
}

MEtl::string ttoa(const OptimizeEnumeration& t){
  int val = t;
  const MEtl::string& strVal = t[val];
  if(strVal == EnumDictionary::INVALID_STR_VAL){
    fprintf(stderr, "can't find matching enumeration for numeric value %d\n", val);
  }
  return strVal;
}

const char* ttot(const OptimizeEnumeration& t){return "OptimizeEnumeration";}


const MEtl::string EnumDictionary::INVALID_STR_VAL = "";
void EnumDictionary::set(std::atomic<EnumDictionary*>& dictionary, const IntToStr& intToStr,bool freeze )
{
  EnumDictionary* expected(NULL);
  EnumDictionary* newVal(new EnumDictionary());
  newVal->init(intToStr,freeze);
  while(!dictionary.compare_exchange_strong(expected, newVal)){
    if(dictionary != NULL){
      delete newVal;
      break;
    }
    expected = NULL;
  }
}

void EnumDictionary::init(const IntToStr& intToStr,bool freeze)
{
  _freeze = freeze;
  for (IntToStrIt it = intToStr.begin(); it != intToStr.end(); it++)
  {
    _strToInt[it->second] = it->first;
    _intToStr[it->first] = it->second;
    _startIntToStr[it->first] = it->second;
  }
}
void EnumDictionary::reset()
{
  _freeze = false;
  _intToStr.clear();
  _strToInt.clear();
  for (IntToStrIt it = _startIntToStr.begin(); it != _startIntToStr.end(); it++)
  {
    _strToInt[it->second] = it->first;
    _intToStr[it->first] = it->second;
  }
}

void EnumDictionary::append(const IntToStr& intToStr, bool freeze)
{
  if(_freeze){
    return;
  }
  _freeze = freeze;
  for (IntToStrIt it = intToStr.begin(); it != intToStr.end(); it++)
  {
    if(_intToStr.find(it->first) == _intToStr.end()){
      _intToStr[it->first] = it->second;
      _strToInt[it->second] = it->first;
    }
  }
}
bool EnumDictionary::isFreeze() const
{
  return _freeze;
}
void EnumDictionary::getListStr(StrContainer& list)const
{
  for (IntToStrIt it = _intToStr.begin(); it != _intToStr.end(); it++)
  {
    list.push_back(it->second);
  }
}

void EnumDictionary::getListInts(IntContainer& list)const
{
  for (IntToStrIt it = _intToStr.begin(); it != _intToStr.end(); it++)
  {
    list.push_back(it->first);
  }
}

MEtl::string EnumDictionary::operator[](int val) const
{
  IntToStrIt it = _intToStr.find(val);
  if(it != _intToStr.end()){
    return it->second;
  }
  return INVALID_STR_VAL;
}

const MEtl::string& EnumDictionary::operator()(int val) const
{
  IntToStrIt it = _intToStr.find(val);
  if(it != _intToStr.end()){
    return it->second;
  }
  return INVALID_STR_VAL;
}

int EnumDictionary::operator[](const MEtl::string& strVal) const{
  StrToIntIt it = _strToInt.find(strVal);
  if(it != _strToInt.end()){
    return it->second;
  }
  return INVALID_VAL;
}
