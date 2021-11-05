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
int page_refence[262144]={0};
struct spinlock mutex;
struct spinlock big_mutex;
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&mutex, "mutex");
  initlock(&mutex, "big_mutex");
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  {
    set_page_reference((uint64)p,1);
    kfree(p);
  }
    
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
  acquire(&big_mutex);  
  int count=add_page_reference((uint64)pa,-1);  
  if(count<0){
    panic("kfree 释放没有分配的页");
    
    // 释放引用计数为0的页面
  }else 
  if(count==0){
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  r = (struct run*)pa;
  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
  }
  release(&big_mutex);
  
}
int get_page_reference(uint64 addr){
  acquire(&mutex);
  int index=(PHYSTOP -PGROUNDDOWN(addr))/PGSIZE;
  // printf("index:%d %p\n",index,addr);
  int count=page_refence[index];
  release(&mutex);
  return count;
}
int get_page_index(uint64 addr){
  int index=(PHYSTOP -PGROUNDDOWN(addr))/PGSIZE;
  return index;
}
void set_page_reference(uint64 addr,int val){
  acquire(&mutex);
  int index=(PHYSTOP -PGROUNDDOWN(addr))/PGSIZE;
  page_refence[index]=val;
  release(&mutex);
}
int add_page_reference(uint64 addr,int count){
  acquire(&mutex);
  int index=(PHYSTOP -PGROUNDDOWN(addr))/PGSIZE;
  page_refence[index]+=count;
  count =page_refence[index];
  release(&mutex);
  return count;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  //将新分配的页的引用计数设置为1; 
  set_page_reference((uint64)r,1);
  
  return (void*)r;
}
