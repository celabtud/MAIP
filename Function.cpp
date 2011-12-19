// Function.cpp
#include <pin.H>

#include "Function.h"


Function::Function(const string &pName)
{
    m_name = pName;
    m_mem_info = new MemInfo();
    m_current_mem_info = new MemInfo();
    m_called = 0;
    m_monitored = false;
    m_first = true;
    m_source = "unknown";
}

Function::Function(const string &pName, bool pMonitored)
{
    m_name = pName;
    m_mem_info = new MemInfo();
    m_current_mem_info = new MemInfo();
    m_called = 0;
    m_monitored = pMonitored;
    m_first = true;
    m_source = "unknown";
}

Function::~Function()
{
    delete m_mem_info;
    delete m_current_mem_info;
}

void Function::processInstruction(int size)
{
    m_current_mem_info->GlobalTotalInst()++;
    m_current_mem_info->TotalInst()++;
    m_current_mem_info->TotalOps() += size;
}

void Function::processOperand(void * ip, void * addr, void * esp, UINT32 size, bool isread)
{
    m_current_mem_info->processOperand(ip,addr,esp,size,isread);
}

void Function::print(ostream &of)
{
    of << "==========================================================================="<<endl;
    of << endl <<"Function name: "<< m_name <<endl;
    of << "Source name: "<< m_source <<endl;
    of<<"Total number of times this function was called: " << m_called <<endl;
    of.precision(10);
    of<<"Avg. instructions per call: " <<((double)m_mem_info->TotalInst())/((double)m_called) <<endl;

    m_mem_info->print(of);
    
    of << endl;

}

void Function::updateMemInfo(void * ip, void * esp, char r, void * addr, int32_t size, bool isprefetch, bool sameinst)
{
    m_current_mem_info->update(ip,esp,r,addr,size,isprefetch,sameinst);	
} 

void Function::processCall(bool final)
{
    // do update
    if(!final) m_called++;
    if(!m_first) {
        *m_mem_info += *m_current_mem_info;

        m_mem_info->updateMax(*m_current_mem_info);
        m_mem_info->updateMin(*m_current_mem_info);
    }
    m_first = false;
    // reset current node
    m_current_mem_info->reset();
}
