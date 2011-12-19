// MemInfo.cpp

#include "MemInfo.h"
#include <limits>


// static initialization
int64_t MemInfo::g_totalinst = 0;


MemInfo::MemInfo()
{
    reset();
}

MemInfo::~MemInfo()
{
}


void MemInfo::reset()
{
    totalinst = 0;
    totalops = 0;

    // total number of r/w instructions
    total_memory_read_ins_stack = 0;
    total_memory_read_ins_nostack = 0;
    total_memory_write_ins_stack = 0;
    total_memory_write_ins_nostack = 0;
    // total number of bytes accessed in r/w
    memory_read_stack_bytes = 0;
    memory_read_nostack_bytes = 0;
    memory_write_stack_bytes = 0;
    memory_write_nostack_bytes = 0;
    // total number of operands in r/w instructions
    total_memory_read_operand_stack = 0;
    total_memory_read_operand_nostack = 0;
    total_memory_write_operand_stack = 0;
    total_memory_write_operand_nostack = 0;

    min_mai = numeric_limits<long double>::max();
    min_heap_mai = numeric_limits<long double>::max();
    min_moi = numeric_limits<long double>::max();
    min_heap_moi = numeric_limits<long double>::max();
    min_stack = numeric_limits<long double>::max();
    min_flow = numeric_limits<long double>::max();
    min_heap_flow = numeric_limits<long double>::max();
    min_bpa = numeric_limits<long double>::max();

    max_mai = -numeric_limits<long double>::max();
    max_heap_mai = -numeric_limits<long double>::max();
    max_moi = -numeric_limits<long double>::max();
    max_heap_moi = -numeric_limits<long double>::max();
    max_stack = -numeric_limits<long double>::max();
    max_flow = -numeric_limits<long double>::max();
    max_heap_flow = -numeric_limits<long double>::max();
    max_bpa = -numeric_limits<long double>::max();
}

MemInfo &MemInfo::operator+=(const MemInfo& other)
{

    totalinst += other.totalinst;
    totalops += other.totalops;
    // total number of r/w instructions
    total_memory_read_ins_stack += other.total_memory_read_ins_stack;
    total_memory_read_ins_nostack += other.total_memory_read_ins_nostack;
    total_memory_write_ins_stack += other.total_memory_write_ins_stack;
    total_memory_write_ins_nostack += other.total_memory_write_ins_nostack;
    // total number of bytes accessed in r/w
    memory_read_stack_bytes += other.memory_read_stack_bytes;
    memory_read_nostack_bytes += other.memory_read_nostack_bytes;
    memory_write_stack_bytes += other.memory_write_stack_bytes;
    memory_write_nostack_bytes += other.memory_write_nostack_bytes;
    // total number of operands in r/w instructions
    total_memory_read_operand_stack += other.total_memory_read_operand_stack;
    total_memory_read_operand_nostack += other.total_memory_read_operand_nostack;
    total_memory_write_operand_stack += other.total_memory_write_operand_stack;
    total_memory_write_operand_nostack += other.total_memory_write_operand_nostack;
    // total number of bytes accessed in operands of the r/w instructions -> should correspond to the total bytes accessed!
    return *this;
}

void MemInfo::updateMax(const MemInfo& other)
{
    max_mai = max(max_mai, other.getMAIIndex());
    max_heap_mai = max(max_heap_mai, other.getHeapMAIIndex());
    max_moi = max(max_moi, other.getMOIIndex());
    max_heap_moi = max(max_heap_moi, other.getHeapMOIIndex());
    max_stack = max(max_stack, other.getStackRatio());
    max_flow = max(max_flow, other.getFlowRatio());
    max_heap_flow = max(max_heap_flow, other.getHeapFlowRatio());
    max_bpa = max(max_bpa, other.getBytesPerAccess());
}

void MemInfo::updateMin(const MemInfo& other)
{
    static bool firsttime = true;

    min_mai = min(min_mai, other.getMAIIndex());
    min_heap_mai = min(min_heap_mai, other.getHeapMAIIndex());
    min_moi = min(min_moi, other.getMOIIndex());
    min_heap_moi = min(min_heap_moi, other.getHeapMOIIndex());
    min_stack = min(min_stack, other.getStackRatio());
    min_flow = min(min_flow, other.getFlowRatio());
    min_heap_flow = min(min_heap_flow, other.getHeapFlowRatio());
    min_bpa = min(min_bpa, other.getBytesPerAccess());
    firsttime = false;
}

// MAI RELATED
int64_t MemInfo::getMemoryReadsIns() const
{
    return total_memory_read_ins_stack + total_memory_read_ins_nostack;
}
int64_t MemInfo::getMemoryWritesIns() const
{
    return total_memory_write_ins_stack + total_memory_write_ins_nostack;
}

int64_t MemInfo::getStackIns() const
{
    return total_memory_read_ins_stack + total_memory_write_ins_stack;
}
int64_t MemInfo::getMemoryIns() const
{
    return getMemoryReadsIns() + getMemoryWritesIns();
}

int64_t MemInfo::getHeapMemoryIns() const
{
    return total_memory_write_ins_nostack + total_memory_read_ins_nostack;
}

// Memory Access Intensity (Instruction)
long double MemInfo::getMAIIndex() const
{
    return (long double)getMemoryIns()/(long double)getTotalIns();
}

long double MemInfo::getHeapMAIIndex() const
{
    return (long double)getHeapMemoryIns()/(long double)getTotalIns();
}



// MOI RELATED
int64_t MemInfo::getMemoryReadsOps() const
{
    return total_memory_read_operand_stack + total_memory_read_operand_nostack;
}
int64_t MemInfo::getMemoryWritesOps() const
{
    return total_memory_write_operand_stack + total_memory_write_operand_nostack;
}
int64_t MemInfo::getStackOps() const
{
    return total_memory_read_operand_stack + total_memory_write_operand_stack;
}
int64_t MemInfo::getMemoryOps() const
{
    return getMemoryReadsOps() + getMemoryWritesOps();
}

int64_t MemInfo::getHeapMemoryOps() const
{
    return total_memory_write_operand_nostack + total_memory_read_operand_nostack;
}

// Memory Access Intensity (Operands)
long double MemInfo::getMOIIndex() const
{
    return (long double)getMemoryOps()/(long double)getTotalOps();
}

long double MemInfo::getHeapMOIIndex() const
{
    return (long double)getHeapMemoryOps()/(long double)getTotalOps();
}


// bytewise Stack and flow ratios

int64_t MemInfo::getMemoryReadsBytes() const
{
    return memory_read_stack_bytes + memory_read_nostack_bytes;
}
int64_t MemInfo::getMemoryWritesBytes() const
{
    return memory_write_stack_bytes + memory_write_nostack_bytes;
}
int64_t MemInfo::getStackBytes() const
{
    return memory_read_stack_bytes + memory_write_stack_bytes;
}
int64_t MemInfo::getHeapBytes() const
{
    return memory_read_nostack_bytes + memory_write_nostack_bytes;
}

int64_t MemInfo::getMemoryBytes() const
{
    return getMemoryReadsBytes() + getMemoryWritesBytes();
}

int64_t MemInfo::getHeapMemoryBytes() const
{
    return memory_write_nostack_bytes + memory_read_nostack_bytes;
}


long double MemInfo::getStackRatio() const
{
    return (long double)getStackBytes() / (long double)getMemoryBytes();
}

long double MemInfo::getFlowRatio() const
{
    return (long double)(getMemoryReadsBytes()-getMemoryWritesBytes())/(long double)(getMemoryBytes());
}

long double MemInfo::getHeapFlowRatio() const
{
    int64_t bytes = getHeapMemoryBytes();

    if(bytes==0) 
        return 0.0;

    return (long double)(memory_read_nostack_bytes - memory_write_nostack_bytes)/(long double)(bytes);
}


// other
long double MemInfo::getBytesPerAccess() const
{
    return (long double)getMemoryBytes()/(long double)getMemoryOps();
}


long double MemInfo::getContribution() const
{
    return (long double)totalinst/(long double)g_totalinst;
}

void MemInfo::print(ostream &of)
{
    of << "RAW MEMORY STATISTICS:" << endl;
    of << "Computational Contribution: " << getContribution() << endl;
    of << "Total # of Instructions executed: " << totalinst << endl;
    of << "Total # of Memory Access Instructions executed: " << getMemoryIns() << endl;
    of << "Total # of Stack Access Instructions executed: " << getStackIns() << endl;
    of << "Total # of Heap Access Instructions executed: " << getHeapMemoryIns() << endl;

    of << "Total # of Memory reads: " << getMemoryReadsIns() << endl;
    of << "Total # of Stack reads: " << total_memory_read_ins_stack << endl;
    of << "Total # of Heap reads: " << total_memory_read_ins_nostack << endl;
    of << "Total # of Memory writes: " << getMemoryWritesIns() << endl;
    of << "Total # of Stack writes: " << total_memory_write_ins_stack << endl;
    of << "Total # of Heap writes: " << total_memory_write_ins_nostack << endl;

    of << "Total # of Memory bytes accessed: " << getMemoryBytes() << endl;
    of << "Total # of Stack bytes accessed: " << getStackBytes() << endl;
    of << "Total # of Heap bytes accessed: " << getHeapBytes() << endl;
    of << "Total # of Memory bytes read: " << getMemoryReadsBytes() << endl;
    of << "Total # of Stack bytes read: " << memory_read_stack_bytes << endl;
    of << "Total # of Heap bytes read: " << memory_read_nostack_bytes << endl;
    of << "Total # of Memory bytes written: " << getMemoryWritesBytes() << endl;
    of << "Total # of Stack bytes written: " << memory_write_stack_bytes << endl;
    of << "Total # of Heap bytes written: " << memory_write_nostack_bytes << endl;

    of << "PROCESSED MEMORY STATISTICS:" << endl;
    of << "Avg MAR Index       : " << getMAIIndex() << endl;
    of << "Avg Heap MAR Index  : " << getHeapMAIIndex() << endl;
    of << "Avg MOR Index       : " << getMOIIndex() << endl;
    of << "Avg Heap MOR Index  : " << getHeapMOIIndex() << endl;
    of << "Avg Stack Ratio     : " << getStackRatio() << endl;
    of << "Avg Flow Ratio      : " << getFlowRatio() << endl;
    of << "Avg Heap Flow Ratio : " << getHeapFlowRatio() << endl;
    of << "Avg Bytes Per Access: " << getBytesPerAccess() << endl;

    of << "Max MAR Index       : " << getMaxMAIIndex() << endl;
    of << "Max Heap MAR Index  : " << getMaxHeapMAIIndex() << endl;
    of << "Max MOR Index       : " << getMaxMOIIndex() << endl;
    of << "Max Heap MOR Index  : " << getMaxHeapMOIIndex() << endl;
    of << "Max Stack Ratio     : " << getMaxStackRatio() << endl;
    of << "Max Flow Ratio      : " << getMaxFlowRatio() << endl;
    of << "Max Heap Flow Ratio : " << getMaxHeapFlowRatio() << endl;
    of << "Max Bytes Per Access: " << getMaxBytesPerAccess() << endl;

    of << "Min MAR Index       : " << getMinMAIIndex() << endl;
    of << "Min Heap MAR Index  : " << getMinHeapMAIIndex() << endl;
    of << "Min MOR Index       : " << getMinMOIIndex() << endl;
    of << "Min Heap MOR Index  : " << getMinHeapMOIIndex() << endl;
    of << "Min Stack Ratio     : " << getMinStackRatio() << endl;
    of << "Min Flow Ratio      : " << getMinFlowRatio() << endl;
    of << "Min Heap Flow Ratio : " << getMinHeapFlowRatio() << endl;
    of << "Min Bytes Per Access: " << getMinBytesPerAccess() << endl;
}




void MemInfo::update(void * ip, void * esp, char r, void * addr, int size, bool isprefetch, bool sameinst)
{
	//reading *addr (with respect to the size should reveal the value read or written)
	// not checking prefetch at the moment!
	// the ip is also not used here!

	if (r=='R')
	{
		if (addr >= esp) // stack region
		{
			if(!sameinst) // for the second read do not increment the number of instructions
				total_memory_read_ins_stack++;
			
			memory_read_stack_bytes+=size;
		}
		else 
		{
			if(!sameinst) // for the second read do not increment the number of instructions
				total_memory_read_ins_nostack++;
			
			memory_read_nostack_bytes+=size;
		}
	}
	else // r=='w'
	{
		if (addr >= esp) // stack region
		{
			if(!sameinst) // for a combined r/w instruction which was counted in the first read do not increment the number of instructions
				total_memory_write_ins_stack++;
			
			memory_write_stack_bytes+=size;
		}
		else 
		{
			if(!sameinst) // for a combined r/w instruction which was counted in the first read do not increment the number of instructions
				total_memory_write_ins_nostack++;
			
			memory_write_nostack_bytes+=size;
		}
	
	} // end of write access
}

void MemInfo::processOperand(void * ip, void * addr, void * esp, uint32_t size, bool isread)
{
    if(isread)
    {
        if (addr >= esp) // stack region
        {
            total_memory_read_operand_stack++;
        }
        else
        {
            total_memory_read_operand_nostack++;
        }
    } else {	
        if (addr >= esp) // stack region
        {
            total_memory_write_operand_stack++;
        }
        else
        {
            total_memory_write_operand_nostack++;
        }	
    }
}
