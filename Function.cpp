// Function.cpp
#include <pin.H>

#include "Function.h"
#include <sys/utsname.h>

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

ticpp::Element * Function::createMeasurementTag(const string& name, double value)
{
    ticpp::Element *measurement = new ticpp::Element(g_namespace + "measurement");

    measurement->SetAttribute("name",name);
    measurement->SetAttribute("type","prediction");
    measurement->SetAttribute("by","MAIP");
    measurement->SetAttribute("unit", "unknown");
    measurement->SetText(value);
    return measurement;
}

struct utsname machine_name_result;
static char *get_machine_name()
{
    uname(&machine_name_result);
    return machine_name_result.machine;
}

void Function::outputXML(ticpp::Element *function)
{
    ticpp::Element *parent;
    ticpp::Element *statistics = function->FirstChildElement(g_namespace + "statistics");
    //get target architecture
    string arch(get_machine_name());
    //find target tag of that architecture
    ticpp::Iterator< ticpp::Element > target(g_namespace + "target");
    for(target = target.begin(statistics);
        target != target.end();
        target++)
        if(target->GetAttribute("name").compare("arch") == 0)
            break;
    if(target != target.end())
        parent = target.Get();
    else {
        // create new target
        parent = new ticpp::Element(g_namespace + "target");
        parent->SetAttribute("name",arch);
        statistics->LinkEndChild(parent);
    }
    //remove all measurements by MAIP
    ticpp::Iterator< ticpp::Element > measurement(g_namespace + "measurement");
    measurement = measurement.begin(parent);
    while(measurement != measurement.end())
    {
        ticpp::Element *tag = (measurement++).Get();
        if(tag->GetAttribute("by").compare("MAIP") == 0)
            parent->RemoveChild(tag);
    }
    parent->LinkEndChild(createMeasurementTag("calls",m_called));
    parent->LinkEndChild(createMeasurementTag("avg. instructions/call",((double)m_mem_info->TotalInst())/((double)m_called)));

    m_mem_info->outputXML(parent);
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
