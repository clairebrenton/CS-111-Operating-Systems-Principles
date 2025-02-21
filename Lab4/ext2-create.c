#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t i16;
typedef int32_t i32;

// added
#define EXT2_SUPER_MAGIC 0xEF53
#define MAX_MOUNT_COUNT 0xFFFF // Or is it 0x7FFF ???
#define EXT2_OS_LINUX 0 

#define BLOCK_SIZE 1024
#define BLOCK_OFFSET(i) (i * BLOCK_SIZE)
#define NUM_BLOCKS 1024
#define NUM_INODES 128

#define LOST_AND_FOUND_INO 11
#define HELLO_WORLD_INO    12
#define HELLO_INO          13
#define LAST_INO           HELLO_INO

#define SUPERBLOCK_BLOCKNO             1
#define BLOCK_GROUP_DESCRIPTOR_BLOCKNO 2
#define BLOCK_BITMAP_BLOCKNO           3
#define INODE_BITMAP_BLOCKNO           4
#define INODE_TABLE_BLOCKNO            5
#define ROOT_DIR_BLOCKNO               21
#define LOST_AND_FOUND_DIR_BLOCKNO     22
#define HELLO_WORLD_FILE_BLOCKNO       23
#define LAST_BLOCK                     HELLO_WORLD_FILE_BLOCKNO

#define NUM_FREE_BLOCKS (NUM_BLOCKS - LAST_BLOCK - 1)
#define NUM_FREE_INODES (NUM_INODES - LAST_INO)

/* http://www.nongnu.org/ext2-doc/ext2.html */
/* http://www.science.smith.edu/~nhowe/262/oldlabs/ext2.html */

#define    EXT2_BAD_INO             1
#define EXT2_ROOT_INO            2
#define EXT2_GOOD_OLD_FIRST_INO 11

#define EXT2_GOOD_OLD_REV 0

#define EXT2_S_IFSOCK 0xC000
#define EXT2_S_IFLNK  0xA000
#define EXT2_S_IFREG  0x8000
#define EXT2_S_IFBLK  0x6000
#define EXT2_S_IFDIR  0x4000
#define EXT2_S_IFCHR  0x2000
#define EXT2_S_IFIFO  0x1000
#define EXT2_S_ISUID  0x0800
#define EXT2_S_ISGID  0x0400
#define EXT2_S_ISVTX  0x0200
#define EXT2_S_IRUSR  0x0100
#define EXT2_S_IWUSR  0x0080
#define EXT2_S_IXUSR  0x0040
#define EXT2_S_IRGRP  0x0020
#define EXT2_S_IWGRP  0x0010
#define EXT2_S_IXGRP  0x0008
#define EXT2_S_IROTH  0x0004
#define EXT2_S_IWOTH  0x0002
#define EXT2_S_IXOTH  0x0001

#define    EXT2_NDIR_BLOCKS 12
#define    EXT2_IND_BLOCK   EXT2_NDIR_BLOCKS
#define    EXT2_DIND_BLOCK  (EXT2_IND_BLOCK + 1)
#define    EXT2_TIND_BLOCK  (EXT2_DIND_BLOCK + 1)
#define    EXT2_N_BLOCKS    (EXT2_TIND_BLOCK + 1)

#define EXT2_NAME_LEN 255

struct ext2_superblock {
    u32 s_inodes_count;
    u32 s_blocks_count;
    u32 s_r_blocks_count;
    u32 s_free_blocks_count;
    u32 s_free_inodes_count;
    u32 s_first_data_block;
    u32 s_log_block_size;
    i32 s_log_frag_size;
    u32 s_blocks_per_group;
    u32 s_frags_per_group;
    u32 s_inodes_per_group;
    u32 s_mtime;
    u32 s_wtime;
    u16 s_mnt_count;
    i16 s_max_mnt_count;
    u16 s_magic;
    u16 s_state;
    u16 s_errors;
    u16 s_minor_rev_level;
    u32 s_lastcheck;
    u32 s_checkinterval;
    u32 s_creator_os;
    u32 s_rev_level;
    u16 s_def_resuid;
    u16 s_def_resgid;
    u32 s_pad[5];
    u8 s_uuid[16];
    u8 s_volume_name[16];
    u32 s_reserved[229];
};

struct ext2_block_group_descriptor
{
    u32 bg_block_bitmap;
    u32 bg_inode_bitmap;
    u32 bg_inode_table;
    u16 bg_free_blocks_count;
    u16 bg_free_inodes_count;
    u16 bg_used_dirs_count;
    u16 bg_pad;
    u32 bg_reserved[3];
};

struct ext2_inode {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks;
    u32 i_flags;
    u32 i_reserved1;
    u32 i_block[EXT2_N_BLOCKS];
    u32 i_version;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u8  i_frag;
    u8  i_fsize;
    u16 i_pad1;
    u32 i_reserved2[2];
};

struct ext2_dir_entry {
    u32 inode;
    u16 rec_len;
    u16 name_len;
    u8  name[EXT2_NAME_LEN];
};

#define errno_exit(str)                                                        \
    do { int err = errno; perror(str); exit(err); } while (0)

#define dir_entry_set(entry, inode_num, str)                                   \
    do {                                                                   \
        char *s = str;                                                 \
        size_t len = strlen(s);                                        \
        entry.inode = inode_num;                                       \
        entry.name_len = len;                                          \
        memcpy(&entry.name, s, len);                                   \
        if ((len % 4) != 0) {                                          \
            entry.rec_len = 12 + len / 4 * 4;                      \
        }                                                              \
        else {                                                         \
            entry.rec_len = 8 + len;                               \
        }                                                              \
    } while (0)

#define dir_entry_write(entry, fd)                                             \
    do {                                                                   \
        size_t size = entry.rec_len;                                   \
        if (write(fd, &entry, size) != size) {                         \
            errno_exit("write");                                   \
        }                                                              \
    } while (0)

u32 get_current_time() {
    time_t t = time(NULL);
    if (t == ((time_t) -1)) {
        errno_exit("time");
    }
    return t;
}

void write_superblock(int fd) {
    off_t off = lseek(fd, BLOCK_OFFSET(1), SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    u32 current_time = get_current_time();

    struct ext2_superblock superblock = {0};

    /* These are intentionally incorrectly set as 0, you should set them
       correctly and delete this comment */
    superblock.s_inodes_count      = NUM_INODES; // macro to be added
    superblock.s_blocks_count      = NUM_BLOCKS;
    superblock.s_r_blocks_count    = 0; // This is fine left as 0
    superblock.s_free_blocks_count = NUM_FREE_BLOCKS; 
    superblock.s_free_inodes_count = NUM_FREE_INODES;
    superblock.s_first_data_block  = SUPERBLOCK_BLOCKNO; /* First Data Block */ // Block 1
    superblock.s_log_block_size    = 0; /* 1024 */
    superblock.s_log_frag_size     = 0; /* 1024 */
    superblock.s_blocks_per_group  = 8*BLOCK_SIZE; // 8 * block sie
    superblock.s_frags_per_group   = 8*BLOCK_SIZE; // Don't worry about this???
    superblock.s_inodes_per_group  = NUM_INODES; 
    superblock.s_mtime             = 0; /* Mount time */
    superblock.s_wtime             = current_time; /* Write time */
    superblock.s_mnt_count         = 0; /* Number of times mounted so far */
    superblock.s_max_mnt_count     = MAX_MOUNT_COUNT; /* Make this unlimited -> in other words the maximum 16-bit number*/
    superblock.s_magic             = EXT2_SUPER_MAGIC; /* ext2 Signature */
    superblock.s_state             = 1; /* File system is clean */
    superblock.s_errors            = 1; /* Ignore the error (continue on) */
    superblock.s_minor_rev_level   = 0; /* Leave this as 0 */
    superblock.s_lastcheck         = current_time; /* Last check time */
    superblock.s_checkinterval     = 1; /* Force checks by making them every 1 second */
    superblock.s_creator_os        = EXT2_OS_LINUX; /* Linux */
    superblock.s_rev_level         = 0; /* Leave this as 0 */
    superblock.s_def_resuid        = 0; /* root */
    superblock.s_def_resgid        = 0; /* root */

    superblock.s_uuid[0] = 0x5A;
    superblock.s_uuid[1] = 0x1E;
    superblock.s_uuid[2] = 0xAB;
    superblock.s_uuid[3] = 0x1E;
    superblock.s_uuid[4] = 0x13;
    superblock.s_uuid[5] = 0x37;
    superblock.s_uuid[6] = 0x13;
    superblock.s_uuid[7] = 0x37;
    superblock.s_uuid[8] = 0x13;
    superblock.s_uuid[9] = 0x37;
    superblock.s_uuid[10] = 0xC0;
    superblock.s_uuid[11] = 0xFF;
    superblock.s_uuid[12] = 0xEE;
    superblock.s_uuid[13] = 0xC0;
    superblock.s_uuid[14] = 0xFF;
    superblock.s_uuid[15] = 0xEE;

    memcpy(&superblock.s_volume_name, "cs111-base", 10);

    ssize_t size = sizeof(superblock);
    if (write(fd, &superblock, size) != size) {
        errno_exit("write");
    }
}

void write_block_group_descriptor_table(int fd) {
    off_t off = lseek(fd, BLOCK_OFFSET(2), SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    struct ext2_block_group_descriptor block_group_descriptor = {0};

    block_group_descriptor.bg_block_bitmap = BLOCK_BITMAP_BLOCKNO;
    block_group_descriptor.bg_inode_bitmap = INODE_BITMAP_BLOCKNO;
    block_group_descriptor.bg_inode_table = INODE_TABLE_BLOCKNO;
    block_group_descriptor.bg_free_blocks_count = NUM_FREE_BLOCKS;
    block_group_descriptor.bg_free_inodes_count = NUM_FREE_INODES;
    block_group_descriptor.bg_used_dirs_count = 2; 

    ssize_t size = sizeof(block_group_descriptor);
    if (write(fd, &block_group_descriptor, size) != size) {
        errno_exit("write");
    }

}

struct bitmap {
    uint8_t bits[1024];
};

void mark_block_used(struct bitmap *b, int block_num){
    int bitnum = block_num - 1;
    b->bits[bitnum / 8] |= (0x01 << (bitnum % 8));
}

void write_block_bitmap(int fd) {
    /* This is all you */
    off_t off = lseek(fd, BLOCK_OFFSET(BLOCK_BITMAP_BLOCKNO), SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    struct bitmap block_bitmap = {0};

    mark_block_used(&block_bitmap, SUPERBLOCK_BLOCKNO);
    mark_block_used(&block_bitmap, BLOCK_GROUP_DESCRIPTOR_BLOCKNO);
    mark_block_used(&block_bitmap, BLOCK_BITMAP_BLOCKNO);
    mark_block_used(&block_bitmap, INODE_BITMAP_BLOCKNO);

    // mark blocks that are allocated to the inode table as used
    int inode_table_total_bytes = (NUM_INODES * 128);
    int inode_table_last_block_size = inode_table_total_bytes % BLOCK_SIZE;
    int inode_table_blocksize;
    if (inode_table_last_block_size != 0){
        inode_table_blocksize = inode_table_total_bytes / BLOCK_SIZE;
    }
    else{
        inode_table_blocksize = inode_table_total_bytes / BLOCK_SIZE + 1;
    }
    for(int i = INODE_TABLE_BLOCKNO; i < INODE_TABLE_BLOCKNO + inode_table_blocksize; i++){
        mark_block_used(&block_bitmap, i);
    }

    mark_block_used(&block_bitmap, ROOT_DIR_BLOCKNO);
    mark_block_used(&block_bitmap, LOST_AND_FOUND_DIR_BLOCKNO);
    mark_block_used(&block_bitmap, HELLO_WORLD_FILE_BLOCKNO);
    
    // mark blocks that can't be represented as used
    for(int i = NUM_BLOCKS; i <= BLOCK_SIZE*8; i++){
        mark_block_used(&block_bitmap, i);
    }

    ssize_t size = sizeof(block_bitmap);
    if (write(fd, &block_bitmap, size) != size) {
        errno_exit("write");
    }

}

void mark_inode_used(struct bitmap *b, int inode_num){
    int bitnum = inode_num-1;
    b->bits[bitnum / 8] |= (0x01 << (bitnum % 8));
}

void write_inode_bitmap(int fd) {
    /* This is all you */
    off_t off = lseek(fd, BLOCK_OFFSET(INODE_BITMAP_BLOCKNO), SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    struct bitmap inode_bitmap = {0};

    mark_inode_used(&inode_bitmap, LOST_AND_FOUND_INO);
    mark_inode_used(&inode_bitmap, HELLO_WORLD_INO);
    mark_inode_used(&inode_bitmap, HELLO_INO);

    // Mark inodes allocated to file system only as used
    for(int i = 1; i < EXT2_GOOD_OLD_FIRST_INO; i++){
        mark_inode_used(&inode_bitmap, i);
    }

    // Mark inodes that can't be represented as used
    for(int i = NUM_INODES+1; i <= 8*BLOCK_SIZE; i++){
        mark_inode_used(&inode_bitmap, i);
    }

    ssize_t size = sizeof(inode_bitmap);
    if (write(fd, &inode_bitmap, size) != size) {
        errno_exit("write");
    }
}

void write_inode(int fd, u32 index, struct ext2_inode *inode) {
    off_t off = BLOCK_OFFSET(INODE_TABLE_BLOCKNO)
                + (index - 1) * sizeof(struct ext2_inode);
    off = lseek(fd, off, SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    ssize_t size = sizeof(struct ext2_inode);
    if (write(fd, inode, size) != size) {
        errno_exit("write");
    }
}

void write_inode_table(int fd) {
    u32 current_time = get_current_time();

    struct ext2_inode lost_and_found_inode = {0};
    lost_and_found_inode.i_mode = EXT2_S_IFDIR
                                  | EXT2_S_IRUSR
                                  | EXT2_S_IWUSR
                                  | EXT2_S_IXUSR
                                  | EXT2_S_IRGRP
                                  | EXT2_S_IXGRP
                                  | EXT2_S_IROTH
                                  | EXT2_S_IXOTH;
    lost_and_found_inode.i_uid = 0;
    lost_and_found_inode.i_size = 1024; 
    lost_and_found_inode.i_atime = current_time;
    lost_and_found_inode.i_ctime = current_time;
    lost_and_found_inode.i_mtime = current_time;
    lost_and_found_inode.i_dtime = 0;
    lost_and_found_inode.i_gid = 0;
    lost_and_found_inode.i_links_count = 2;
    lost_and_found_inode.i_blocks = 2; /* These are oddly 512 blocks */
    lost_and_found_inode.i_block[0] = LOST_AND_FOUND_DIR_BLOCKNO;
    write_inode(fd, LOST_AND_FOUND_INO, &lost_and_found_inode);


    // Root directory inode
    struct ext2_inode root_dir_inode = {0};
    root_dir_inode.i_mode = EXT2_S_IFDIR
                            | EXT2_S_IRUSR
                            | EXT2_S_IWUSR
                            | EXT2_S_IXUSR
                            | EXT2_S_IRGRP
                            | EXT2_S_IXGRP
                            | EXT2_S_IROTH
                            | EXT2_S_IXOTH;
    root_dir_inode.i_size = 1024; 
    root_dir_inode.i_uid = 0;
    root_dir_inode.i_atime = current_time;
    root_dir_inode.i_ctime = current_time;
    root_dir_inode.i_mtime = current_time;
    root_dir_inode.i_dtime = 0;
    root_dir_inode.i_gid = 0;
    root_dir_inode.i_links_count = 3; 
    root_dir_inode.i_blocks = 2; /* These are oddly 512 blocks */
    root_dir_inode.i_block[0] = ROOT_DIR_BLOCKNO;
    write_inode(fd, EXT2_ROOT_INO, &root_dir_inode);

    // Hello world file inode
    struct ext2_inode hello_world_inode = {0};
    hello_world_inode.i_mode = EXT2_S_IFREG
                                | EXT2_S_IRUSR
                                | EXT2_S_IWUSR
                                | EXT2_S_IRGRP
                                | EXT2_S_IROTH;
    hello_world_inode.i_size = 12;
    hello_world_inode.i_uid = 1000;
    hello_world_inode.i_atime = current_time;
    hello_world_inode.i_ctime = current_time;
    hello_world_inode.i_mtime = current_time;
    hello_world_inode.i_dtime = 0;
    hello_world_inode.i_gid = 1000;
    hello_world_inode.i_links_count = 1;
    hello_world_inode.i_blocks = 2; /* These are oddly 512 blocks */ 
    hello_world_inode.i_block[0] = HELLO_WORLD_FILE_BLOCKNO;
    write_inode(fd, HELLO_WORLD_INO, &hello_world_inode);

    // Hello symbolic link inode
    struct ext2_inode hello_inode = {0};
    hello_inode.i_mode = EXT2_S_IFLNK
                        | EXT2_S_IRUSR
                        | EXT2_S_IWUSR
                        | EXT2_S_IRGRP
                        | EXT2_S_IROTH;
    hello_inode.i_size = 11;
    hello_inode.i_uid = 1000;
    hello_inode.i_atime = current_time;
    hello_inode.i_ctime = current_time;
    hello_inode.i_mtime = current_time;
    hello_inode.i_dtime = 0;
    hello_inode.i_gid = 1000;
    hello_inode.i_links_count = 1;
    hello_inode.i_blocks = 0; 

    int stringToAscii(const char* str) {
        u32 result = 0;
        for (int i = 0; i < 4; i++) {
            result |= str[i] << (8 * i);
        }
        return result;
    }

    hello_inode.i_block[0] = stringToAscii("hell");
    hello_inode.i_block[1] = stringToAscii("o-wo");
    hello_inode.i_block[2] = stringToAscii("rld");

    write_inode(fd, HELLO_INO, &hello_inode);

}

void write_root_dir_block(int fd) {
    /* This is all you */
    
    off_t off = BLOCK_OFFSET(ROOT_DIR_BLOCKNO);
    off = lseek(fd, off, SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    ssize_t bytes_remaining = BLOCK_SIZE;

    // Current entry
    struct ext2_dir_entry current_entry = {0};
    dir_entry_set(current_entry, EXT2_ROOT_INO, ".");
    dir_entry_write(current_entry, fd);
    
    bytes_remaining -= current_entry.rec_len;

    // Parent entry
    struct ext2_dir_entry parent_entry = {0};
    dir_entry_set(parent_entry, EXT2_ROOT_INO, "..");
    dir_entry_write(parent_entry, fd);
    
    bytes_remaining -= parent_entry.rec_len;

    // Lost+found entry
    struct ext2_dir_entry lost_and_found_entry = {0};
    dir_entry_set(lost_and_found_entry, LOST_AND_FOUND_INO, "lost+found");
    dir_entry_write(lost_and_found_entry, fd);
    
    bytes_remaining -= lost_and_found_entry.rec_len;

    // Hello-world file entry
    struct ext2_dir_entry hello_world_entry = {0};
    dir_entry_set(hello_world_entry, HELLO_WORLD_INO, "hello-world");
    dir_entry_write(hello_world_entry, fd);
    
    bytes_remaining -= hello_world_entry.rec_len;

    // Hello symlink entry
    struct ext2_dir_entry hello_entry = {0};
    dir_entry_set(hello_entry, HELLO_INO, "hello");
    dir_entry_write(hello_entry, fd);
    
    bytes_remaining -= hello_entry.rec_len;

    struct ext2_dir_entry fill_entry = {0};
    fill_entry.rec_len = bytes_remaining;
    dir_entry_write(fill_entry, fd);
}

void write_lost_and_found_dir_block(int fd) {
    off_t off = BLOCK_OFFSET(LOST_AND_FOUND_DIR_BLOCKNO);
    off = lseek(fd, off, SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    ssize_t bytes_remaining = BLOCK_SIZE;

    struct ext2_dir_entry current_entry = {0};
    dir_entry_set(current_entry, LOST_AND_FOUND_INO, ".");
    dir_entry_write(current_entry, fd);

    bytes_remaining -= current_entry.rec_len;

    struct ext2_dir_entry parent_entry = {0};
    dir_entry_set(parent_entry, EXT2_ROOT_INO, "..");
    dir_entry_write(parent_entry, fd);

    bytes_remaining -= parent_entry.rec_len;

    struct ext2_dir_entry fill_entry = {0};
    fill_entry.rec_len = bytes_remaining;
    dir_entry_write(fill_entry, fd);
}

void write_hello_world_file_block(int fd) {
    /* This is all you */
    off_t off = BLOCK_OFFSET(HELLO_WORLD_FILE_BLOCKNO);
    off = lseek(fd, off, SEEK_SET);
    if (off == -1) {
        errno_exit("lseek");
    }

    char text[] = "Hello world\n"; 
    if (write(fd, &text, sizeof(text)) != sizeof(text)) {
        errno_exit("write");
    }
}

int main(int argc, char *argv[]) {
    int fd = open("cs111-base.img", O_CREAT | O_WRONLY, 0666);
    if (fd == -1) {
        errno_exit("open");
    }

    if (ftruncate(fd, 0)) {
        errno_exit("ftruncate");
    }
    if (ftruncate(fd, NUM_BLOCKS * BLOCK_SIZE)) {
        errno_exit("ftruncate");
    }

    write_superblock(fd);
    write_block_group_descriptor_table(fd);
    write_block_bitmap(fd);
    write_inode_bitmap(fd);
    write_inode_table(fd);
    write_root_dir_block(fd);
    write_lost_and_found_dir_block(fd);
    write_hello_world_file_block(fd);

    if (close(fd)) {
        errno_exit("close");
    }
    return 0;
}

