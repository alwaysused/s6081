// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

//hash refered to https://blog.miigon.net/posts/s081-lab8-locks/
#define NBUFMAP_BUCKET 13
#define BUFMAP_HASH(dev, blockno) ((((dev)<<27)|(blockno))%NBUFMAP_BUCKET)

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf bufmap[NBUFMAP_BUCKET];

  struct spinlock  locks[NBUFMAP_BUCKET];
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  //bcache.head.prev = &bcache.head;
  //bcache.head.next = &bcache.head;
  for(int i = 0;i< NBUFMAP_BUCKET; i++){
//    b->next = bcache.head.next;
//    b->prev = &bcache.head;
    //b = &bcache.buf[i];
    //initsleeplock(&b->lock, "buffer");
      initsleeplock(&bcache->locks[i], "buffer");
      b = bcache->bufmap[i];
      b.next = 0;
//    bcache.head.next->prev = b;
//    bcache.head.next = b;
  }
  
   for(int i=0;i<NBUF;i++){
      struct buf *b = &bcache.buf[i];
      initsleeplock(&b->lock, "buffer");
      b->lastuse = 0;
      b->refcnt = 0;
      // put all the buffers into bufmap[0]
      b->next = bcache.bufmap[0].next;
      bcache.bufmap[0].next = b;
    }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.

struct buffer * findExisted()
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int no = BUFMAP_HASH(dev, blockno);
  //b = &bcache.buf[no];
   accquire(&bcache.locks[no]);
   
     // Is the block already cached?
     for(b = bcache.bufmap[key].next; b; b = b->next){
       if(b->dev == dev && b->blockno == blockno){
         b->refcnt++;
         release(&bcache.locks[key]);
         acquiresleep(&b->lock);
         return b;
       }
     }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
      acquire(&bcache.lock);
    release(&bcache.locks[key]);
    uint min = 10000000;
  int idx = -1;
  uint hold = 0;
  int i;
   struct buf * before = 0;
  for(i = 0; i < NBUFMAP_BUCKET; i ++){
    //b = &bcache.bufmap[i];
     acquire(&bcache.locks[i];

     for(b = bcache.bufmap[i]; b->next; b = b->next){
            if(b->next->refcnt == 0) {  //no need to acquire buf lock,because lru
                  if (b->ticks < min){
                    before = b;
                  }
          }

    }
     release(&bcache.locks[i];


   // releasesleep(&b->lock); // if release ,maybe next come and consume
  }
  if (before->next != -1){
  accquire(&bcache.locks[no]);

    if (i != no){
    b = before->next ;
    accquire(&bcache.locks[i]);
    before->next = before->next->next;
    b->next = bcache.bufmap[no]->next;
    bcache.bufmap[no]->next = b;
    }

    b->dev = dev;
              b->blockno = blockno;
              b->valid = 0;
              b->refcnt = 1;
              b->ticks = ticks;
              if (i != no){
              release(&bcache.locks[i]);
              }
              release(&bcache.locks[no]);
              acquiresleep(&bcache.locks);
                 return b;


    } else{

      panic("bget: no buffers");

    }







}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");
     uint key = BUFMAP_HASH(b->dev, b->blockno);

      acquire(&bcache.locks[key]);
  b->refcnt--;


  //acquire(&bcache.lock);
  if (b->refcnt == 0) {
    // no one is waiting for it.
//    b->next->prev = b->prev;
//    b->prev->next = b->next;
//    b->next = bcache.head.next;
//    b->prev = &bcache.head;
//    bcache.head.next->prev = b;
//    bcache.head.next = b;
    b->ticks = ticks;
  }
  //releasesleep(&b->lock);//why release here?

  //release(&bcache.lock);
    release(&bcache.ocks[key]);

}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


