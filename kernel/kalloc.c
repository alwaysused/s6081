// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

uint64 getcpuId(){
  return cpuid();
}

void
kinit()
{
  for (int i = 0;i < NCPU;i++){
      initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}


void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{

  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  uint64 cpu = cpuid();

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.

void* kallocFromIdx(uint64 cpuid){
 struct run *r=0;
  acquire(&kmem[cpuid].lock);
  r = kmem[cpuid].freelist;
  if(r){
    kmem[cpuid].freelist = r->next;
  }
  release(&kmem[cpuid].lock);
  return (void*)r;

}

void *
kalloc(void)
{
  struct run *r;
  push_off();
  int cpu = cpuid();

  acquire(&kmem[cpu].lock);
 if (!kmem[cpu].freelist){

    for (int i = 0;i < NCPU&&!kmem[cpu].freelist;i++){
    if (i == cpu) continue; //dangshi xiezai xunhuan li chucuole ...
//      struct run* p = kallocFromIdx(i);
//      if (p!=0){
//        p->next = kmem[cpuid].freelist;
//                kmem[cpuid].freelist = p;
//        break;
//      }
      acquire(&kmem[i].lock);
      struct run *rr = kmem[i].freelist;
      if (rr) {
        kmem[i].freelist = rr->next;
        rr->next = kmem[cpu].freelist;
        kmem[cpu].freelist = rr;
      }
      release(&kmem[i].lock);
      }
  }
   r = kmem[cpu].freelist;

  if(r){
       kmem[cpu].freelist = r->next;

     }
    release(&kmem[cpu].lock);
  pop_off();
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
// struct run *r;
//
//  push_off();
//
//  int cpu = cpuid();
//
//  acquire(&kmem[cpu].lock);
//
//  if(!kmem[cpu].freelist) { // no page left for this cpu
//    //int steal_left = 64; // steal 64 pages from other cpu(s)
//    for(int i=0;i<NCPU&&!kmem[cpu].freelist;i++) {
//      if(i == cpu) continue; // no self-robbery
//      acquire(&kmem[i].lock);
//      struct run *rr = kmem[i].freelist;
//      if (rr) {
//        kmem[i].freelist = rr->next;
//        rr->next = kmem[cpu].freelist;
//        kmem[cpu].freelist = rr;
//      }
//      release(&kmem[i].lock);
//    }
//  }
//
//  r = kmem[cpu].freelist;
//  if(r)
//    kmem[cpu].freelist = r->next;
//  release(&kmem[cpu].lock);
//
//  pop_off();
//
//  if(r)
//    memset((char*)r, 5, PGSIZE); // fill with junk
//  return (void*)r;
}
