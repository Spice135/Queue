/******************************************************************************/
/*!
\file   queue.cpp
\author Akshat Madan
\par    email: akshatmadan135@gmail.com
\date   09/15/2021
*/
/******************************************************************************/
#include<iostream> // printf

#define MAX_SIZE 2048 // Max size of data
#define MAX_QUEUES 64 // Max number of queues

typedef short Q; // A queue

const unsigned short byteIndex = sizeof(Q) * MAX_QUEUES;
const unsigned short padding = sizeof(int) - 1;
const unsigned short startOffset = byteIndex + sizeof(short);
unsigned char data[MAX_SIZE]; // Data array

void on_out_of_memory() { printf("Oh No\n");} // Unable to complete request due to lack of memory
void on_illegal_operation() {} // Illegal operation

// Creates a FIFO byte queue, returning a handle to it
Q* create_queue()
{
  static bool isInit = false; // Initialize the data array during first queue creation
  if(!isInit)
  {
    for(unsigned i = 0; i < MAX_QUEUES; ++i)
      *reinterpret_cast<Q*>(&data[sizeof(Q) * i]) = -1; // Make first 128 bytes unused
    *reinterpret_cast<short*>(data + byteIndex) = startOffset; // Number of bytes used
    isInit = true; // Data array has been initialized
  }

  short* bytesUsed = reinterpret_cast<short*>(data + byteIndex);
  if(*bytesUsed >= MAX_SIZE - padding - 2)
    on_out_of_memory(); // Check if memory is available

  Q* size = reinterpret_cast<Q*>(data); // Size of this queue
  Q offset = startOffset; // Offset of new queue
  for(unsigned i = 0; i < MAX_QUEUES; ++i)
  {
    if(*size == -1) // Not been allocated
    {
      float numCopies = float(*bytesUsed - offset) / float(sizeof(int)); // Offset the data by numCopies amount
      int* src = reinterpret_cast<int*>(data + *bytesUsed - sizeof(int)); // Source
      int* dst = reinterpret_cast<int*>(data + *bytesUsed - 1); // Destination
      for(int i = 0; i < numCopies; ++i)
        *(dst--) = *(src--); // Assign & move pointers back
      *size = padding;
      *bytesUsed += padding;
      return size; // Return new queue pointer
    }
    else
      offset += *(size++); // If already allocated, add size to offset
  }
  on_illegal_operation(); // No free queue found
  return nullptr;
}

// Destroy an earlier created byte queue
void destroy_queue(Q * q)
{
  if(!q || *q == -1)
    on_illegal_operation(); // Null or unused queue
  short* bytesUsed = reinterpret_cast<short*>(data + byteIndex); // Number of bytes used
  Q* size = reinterpret_cast<Q*>(data);
  Q offset = startOffset;
  while(size != q)
  {
    if(*size > 0)
      offset += *size; // Find where queue begins
    ++size;
  }
  float numCopies = float(*bytesUsed - offset - *q) / float(sizeof(int));
  int* src = reinterpret_cast<int*>(data + offset + *q); // Source
  int* dst = reinterpret_cast<int*>(data + offset); // Destination
  for(int i = 0; i < numCopies; ++i)
    *(dst++) = *(src++); // Assign & move pointers forward
  *bytesUsed -= *q; // Decrease size by the size of deleted queue
  *q = -1; // Set queue to unused
}

// Adds a new byte to a queue
void enqueue_byte(Q * q, unsigned char b)
{
  if(!q)
    on_illegal_operation(); // Null queue
  short* bytesUsed = reinterpret_cast<short*>(data + byteIndex);
  if(*bytesUsed >= MAX_SIZE)
    on_out_of_memory(); // No memory left to add byte

  Q* size = reinterpret_cast<Q*>(data);
  Q offset = startOffset;
  while(size != q)
  {
    if(*size > 0)
      offset += *size; // Find where queue begins
    size++;
  }
  offset += *size; // First free byte in queue
  float numCopies = float(*bytesUsed - offset) / float(sizeof(int));
  int* src = reinterpret_cast<int*>(data + *bytesUsed - sizeof(int)); // Source
  int* dst = reinterpret_cast<int*>(data + *bytesUsed - (sizeof(int) - 1)); // Destination
  for(int i = 0; i < numCopies; ++i)
    *(dst--) = *(src--); // Assign & move pointers back
  data[offset - padding] = b; // Insert char
  *bytesUsed += 1; // New byte added
  *q += 1; // Queue size increased
}

// Pops the next byte off the FIFO queue
unsigned char dequeue_byte(Q * q)
{
  if(!q || *q <= padding)
    on_illegal_operation(); // Null or empty queue

  short* bytesUsed = reinterpret_cast<short*>(data + byteIndex);
  Q* size = reinterpret_cast<Q*>(data);
  Q offset = startOffset;
  unsigned char retChar; 
  while(size != q)
  {
    if(*size > 0)
      offset += *size; // Find data
    size++;
  }
  retChar = data[offset]; // Char to be popped
  float numCopies = float(*bytesUsed - offset - 1) / float(sizeof(int)); 
  int* src = reinterpret_cast<int*>(data + offset + 1); // Source
  int* dst = reinterpret_cast<int*>(data + offset); // Destination
  for(int i = 0; i < numCopies; ++i)
    *(dst++) = *(src++);
  *bytesUsed -= 1; // Byte freed
  *q -= 1; // Queue size decreased
  return retChar;
}

int main()
{
  Q * q0 = create_queue();
  enqueue_byte(q0, 0);
  enqueue_byte(q0, 1);
  Q * q1 = create_queue();
  enqueue_byte(q1, 3);
  enqueue_byte(q0, 2);
  enqueue_byte(q1, 4);
  printf("%d", dequeue_byte(q0));
  printf(" %d\n", dequeue_byte(q0));
  enqueue_byte(q0, 5);
  enqueue_byte(q1, 6);
  printf("%d", dequeue_byte(q0));
  printf(" %d\n", dequeue_byte(q0));
  destroy_queue(q0);
  printf("%d", dequeue_byte(q1));
  printf(" %d", dequeue_byte(q1));
  printf(" %d\n", dequeue_byte(q1));
  destroy_queue(q1);
  return 0;
}
