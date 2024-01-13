#include <fs.h>
// #include <unistd.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode){
  for (size_t fd = 0; fd < sizeof(file_table) / sizeof(file_table[0]); fd++)
  {
    if(strcmp(file_table[fd].name,pathname)==0) {
      file_table[fd].open_offset = 0; //restore open offset
      Log("open fd=%d",fd);
      return fd;
    }
  }
  Log("pahtname incorrect or file not updated in ramdisk");
  assert(0);
}

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t fs_read(int fd, void *buf, size_t len){
  if(fd == 0 || fd == 1 || fd == 2) {//stdin/stout/stderr
    return 0; //ignored
  }
  size_t read_offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  if(file_table[fd].open_offset+len>file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset; //read till EOF
  }
  ramdisk_read(buf,read_offset,len);
  file_table[fd].open_offset += len;
  return len;
}
size_t fs_write(int fd, const void *buf, size_t len){
  assert(fd>2);
  size_t wr_offset = file_table[fd].disk_offset + file_table[fd].open_offset;
  if(file_table[fd].open_offset+len>file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset; //write till EOF
  }
  ramdisk_write(buf,wr_offset,len);
  file_table[fd].open_offset += len;
  return len;
}


int fs_close(int fd){
  return 0; //note the open_offset is reset when opening file.
}

size_t fs_lseek(int fd, int offset, int whence) {
  if(whence == SEEK_SET) {
    file_table[fd].open_offset = offset;
  }
  else if (whence == SEEK_CUR)
  {
    file_table[fd].open_offset += offset;
  }
  else if(whence == SEEK_END) {
    file_table[fd].open_offset = file_table[fd].size + offset; //don't understand why allow go over the boundry TODO FIXME BOOKMARK
  }
  else return -1;

  return file_table[fd].open_offset;
  
}