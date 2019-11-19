#include <conn.h>
#include <memory.h>

#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <cstring>
#include <mqueue.h>

#define QUEUE_NAME "/LAB2_QUEUE"
#define QUEUE_MAXMSG  1 /* Maximum number of messages. */
#define QUEUE_MSGSIZE sizeof(Memory) /* Length of message. */
#define QUEUE_ATTR_INITIALIZER ((struct mq_attr){0, QUEUE_MAXMSG, QUEUE_MSGSIZE, 0, {0}})

mqd_t mqid;
bool need_to_rm;

Conn::Conn(size_t id, bool create)
{
  need_to_rm = create;
  int mqflg = O_RDWR;
  int mqperm = 0666;
  if (create)
  {
    std::cout << "Creating connection with id = " << id << std::endl;
    mqflg |= O_CREAT | O_NONBLOCK;
    struct mq_attr attr = QUEUE_ATTR_INITIALIZER;
    mqid = mq_open(QUEUE_NAME, mqflg, mqperm, &attr);
  }
  else
  {
    std::cout << "Getting connection with id = " << id << std::endl;
    mqid = mq_open(QUEUE_NAME, mqflg);
  }
  if (mqid == -1)
  {
    std::cout << "ERROR: mq_open failed, errno = " << strerror(errno) << std::endl;
  }
  else
  {
    std::cout << "mq_open returned id = " << mqid << std::endl;
  }
}

bool Conn::Read(void* buf, size_t count)
{
  std::cout << "Conn::Read()" << std::endl;
  Memory mq_buf;
  bool success = true;
  if (mq_receive(mqid, (char *)&mq_buf, QUEUE_MSGSIZE, nullptr) == -1)
  {
    std::cout << "ERROR: mq_recieve failed, errno = " << errno << std::endl;
    success = false;
  }
  else
  {
    *((Memory*) buf) = mq_buf;
  }
  std::cout << "exit Conn::Read()" << std::endl;
  return success;
}

bool Conn::Write(void* buf, size_t count)
{
  std::cout << "Conn::Write()" << std::endl;
  Memory shm_buf;
  bool success = false;
  if (count <= QUEUE_MSGSIZE)
  {
    shm_buf = ((Memory*) buf)->status;
    if (mq_send(mqid, (char*) &shm_buf, count, 0) == -1)
    {
      std::cout << "ERROR: mq_send failed, errno = " << errno << std::endl;
    }
    else
    {
      success = true;
    }
  }
  std::cout << "exit Conn::Write()" << std::endl;
  return success;
}

Conn::~Conn()
{
  std::cout << "Conn::~Conn()" << std::endl;
  mq_close(mqid);
  if (need_to_rm)
  {
    mq_unlink(QUEUE_NAME);
  }
}
