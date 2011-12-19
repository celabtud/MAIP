#ifndef   __MEMINFO_H
#define   __MEMINFO_H

#include <stdint.h>
#include <iostream>

using namespace std;

    

class MemInfo
{
private:
    static int64_t g_totalinst;
    int64_t totalinst;
    int64_t totalops;

    // total number of r/w instructions
    int64_t total_memory_read_ins_stack;
    int64_t total_memory_read_ins_nostack;
    int64_t total_memory_write_ins_stack;
    int64_t total_memory_write_ins_nostack;
    // total number of bytes accessed in r/w
    int64_t memory_read_stack_bytes;
    int64_t memory_read_nostack_bytes;
    int64_t memory_write_stack_bytes;
    int64_t memory_write_nostack_bytes;
    // total number of operands in r/w instructions
    int64_t total_memory_read_operand_stack;
    int64_t total_memory_read_operand_nostack;
    int64_t total_memory_write_operand_stack;
    int64_t total_memory_write_operand_nostack;

    long double min_mai;
    long double min_heap_mai;
    long double min_moi;
    long double min_heap_moi;
    long double min_stack;
    long double min_flow;
    long double min_heap_flow;
    long double min_bpa;

    long double max_mai;
    long double max_heap_mai;
    long double max_moi;
    long double max_heap_moi;
    long double max_stack;
    long double max_flow;
    long double max_heap_flow;
    long double max_bpa;

public:
    MemInfo();
    virtual ~MemInfo();
    
    void reset();

    // simple getters
    static int64_t getGlobalTotalIns() { return MemInfo::g_totalinst; } 
    static int64_t getGlobalTotalInst()  { return MemInfo::g_totalinst; } 
    static int64_t &GlobalTotalInst()   { return MemInfo::g_totalinst; } 
    int64_t getTotalIns() const  { return totalinst; } 
    int64_t getTotalInst() const  { return totalinst; } 
    int64_t &TotalInst()   { return totalinst; } 
    int64_t getTotalOps() const  { return totalops; }
    int64_t &TotalOps()  { return totalops; }

    // update
    void update(void * ip, void * esp, char r, void * addr, int32_t size, bool isprefetch, bool sameinst);
    MemInfo &operator+=(const MemInfo& other);
    void updateMax(const MemInfo& other);
    void updateMin(const MemInfo& other);


    void processOperand(void * ip, void * addr, void * ESP, uint32_t size, bool isread);

    // Instruction-wise helper functions
    int64_t getMemoryReadsIns() const;
    int64_t getMemoryWritesIns() const;
    int64_t getStackIns() const;
    int64_t getMemoryIns() const;
    int64_t getHeapMemoryIns() const;
    
    // Instruction-wise indices
    long double getMAIIndex() const;
    long double getHeapMAIIndex() const;
    long double getMinMAIIndex() const { return min_mai; }
    long double getMinHeapMAIIndex() const { return min_heap_mai; }
    long double getMaxMAIIndex() const { return max_mai; }
    long double getMaxHeapMAIIndex() const { return max_heap_mai; }

    // Operand-wise helper functions
    int64_t getMemoryReadsOps() const;
    int64_t getMemoryWritesOps() const;
    int64_t getStackOps() const;
    int64_t getMemoryOps() const;
    int64_t getHeapMemoryOps() const;
    
    // Operand-wise indices
    long double getMOIIndex() const;
    long double getHeapMOIIndex() const;
    long double getMinMOIIndex() const { return min_moi; }
    long double getMinHeapMOIIndex() const { return min_heap_moi; }
    long double getMaxMOIIndex() const { return max_moi; }
    long double getMaxHeapMOIIndex() const { return max_heap_moi; }

    // bytewise helper functions
    int64_t getMemoryReadsBytes() const;
    int64_t getMemoryWritesBytes() const;
    int64_t getStackBytes() const;
    int64_t getHeapBytes() const;
    int64_t getMemoryBytes() const;
    int64_t getHeapMemoryBytes() const;

    // ratios
    long double getStackRatio() const;
    long double getFlowRatio() const;
    long double getHeapFlowRatio() const;
    long double getBytesPerAccess() const;

    long double getMinStackRatio() const { return min_stack; }
    long double getMinFlowRatio() const { return min_flow; }
    long double getMinHeapFlowRatio() const { return min_heap_flow; }
    long double getMinBytesPerAccess() const { return min_bpa; }

    long double getMaxStackRatio() const { return max_stack; }
    long double getMaxFlowRatio() const { return max_flow; }
    long double getMaxHeapFlowRatio() const { return max_heap_flow; }
    long double getMaxBytesPerAccess() const { return max_bpa; }

    long double getContribution() const;

    void print(ostream &of);
}; 
  

#endif // __MEMINFO_H
