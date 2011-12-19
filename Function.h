#ifndef   __FUNCTION_H
#define   __FUNCTION_H

#include <string>
#include <iostream>

#include "MemInfo.h"

using namespace std; 

class Function
{
private:
    string m_name;
    MemInfo *m_mem_info;
    MemInfo *m_current_mem_info;
    int m_called;
    bool m_monitored;
    bool m_first;
    string m_source;

public:
    Function(const string& name);
    Function(const string& name, bool isread);

    virtual ~Function();
    
    string getName() const   { return m_name; }
    string &Name()      { return m_name; }

    string getSource() const   { return m_source; }
    string &Source()      { return m_source; }

    MemInfo *getMemInfo() { return m_mem_info; }    

    int getCalled() const { return m_called; }
    int &Called() { return m_called; }
    
    bool isMonitored() const { return m_monitored; }

    void processInstruction(int size);
    void processOperand(void * ip, void * addr, void * ESP, uint32_t size, bool isread);
    void processCall(bool final = false);

    void print(ostream &of);
    void updateMemInfo(void * ip, void * esp, char r, void * addr, int32_t size, bool isprefetch, bool sameinst);

};
#endif // __FUNCTION_H
