#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include </home/varun/Documents/osproject2/ext2_fs.h>
// #include <linux/ext2_fs.h>
#define NAMELEN     255

static read_superblock(int fd);
static read_groupdesc(int fd, int block_size);
static read_inodetable(int fd, int root_inode_block);
static read_directory_entries(int fd, int block_size);
static unsigned int Inode_table_start; 
int calc_inode_offset(int inode);
static unsigned int block_size = 0;
char *path_to_directory;
static unsigned int bpg;
static unsigned int ipg;
static unsigned int ipb;
int start_addrs_group_desc; 
int root_inode_block;
int directory_block_size;   
static unsigned int start_inode_table;
char dir_parts[15][25];
unsigned int parts_length;
unsigned int inode_mtime;
int inode_uid;
unsigned int inode_size;
int command_option;
int inode_imode;
char permissions[10];

// Usage:  "./display_dir file_name directory_path"
int main(int argc, char **argv) 
{
	const char delim[2] = "/";
  	// open the ext2 file 
	  if ((argc > 2) && (argc < 5)) 
	  {
		    
	  }
	  else
	  {
	  		printf("usage:  ./display_dir diskfile_name path_to_directory command_option\n");
		    return -1;
	  }

  	int fd = open(argv[1], O_RDONLY);

	  if (fd == -1) 
	  {
		    printf("couldn't open file \"%s\"\n", argv[1]);
		    return -1;
		}

	path_to_directory = argv[2];
	if (argc == 4)
	{
		if(strcmp(argv[3], "-d") == 0)
			command_option = 2;
		else
			command_option = 0;
	}
	
	
	//printf("command_option : %d\n", command_option );

	// printf(" path to Directory : %s\n", path_to_directory);
	

	// Finding tokens in the path to directory argument
	char *token = strtok(argv[2], delim);
    int noOfTokens=0;
    /* Getting all other tokens */
    while( token != NULL ) 
    {
    	strcpy(dir_parts[noOfTokens++],token);
        token = strtok(NULL, delim);
        //printf("value %s \n",token);
    }
    parts_length = noOfTokens;	

	// Sending file descriptor as parameter to read Blocks
	 read_superblock(fd);
	 read_groupdesc(fd,start_addrs_group_desc);
	 read_inodetable(fd, root_inode_block);
	 read_directory_entries(fd, directory_block_size);
}


// Reading SUPER BLOCK
read_superblock(int fd)
{
	struct ext2_super_block superblock;		// Creating a structure to read SUPERBLOCK

	// lseek to 1024 address(since boot block occupies first 1024 bytes) and read the super block
	lseek(fd, 1024, SEEK_CUR);
	read(fd, &superblock, sizeof(superblock));

	// Condition to check whether the file we are reading is an ext2 FILE SYSTEM or not
	if (superblock.s_magic != EXT2_SUPER_MAGIC)
	{
		printf(" \n *** Not a ext2 file or Directory *** \n\n");
		exit(1);		
	}

	// Reading the Group Descriptor Block
	block_size = 1024 << superblock.s_log_block_size;

	// Storing information into global variables
	bpg = superblock.s_blocks_per_group;
	ipg = superblock.s_inodes_per_group;
	ipb = block_size / sizeof(struct ext2_inode);

	// Calculating Starting address of group descriptor block
	start_addrs_group_desc = block_size;
}




// Reading Group Descriptor Block

read_groupdesc(int fd, int start_addrs_group_desc)
{
	struct ext2_group_desc groupdesc;	// Creating a new structure of ext2_group_desc


	lseek(fd, start_addrs_group_desc, SEEK_SET);		// lseek to the start of the block address of group descriptor block 
	read(fd, &groupdesc, sizeof(groupdesc));	// reading the group descriptor

	// Reading the Root Inode Information
	Inode_table_start = block_size*groupdesc.bg_inode_table;
	root_inode_block = ((block_size*groupdesc.bg_inode_table) + sizeof(struct ext2_inode));	
	start_inode_table = groupdesc.bg_inode_table;
	
}


// Inodes table 
// Rote Inode Information
read_inodetable(int fd, int root_inode_block)
{
	struct ext2_inode inodetable;	// Creating a new structure of ext2_inode

	lseek(fd, root_inode_block, SEEK_SET);
	read(fd, &inodetable, sizeof(inodetable));		// Reading the inodes table block

	directory_block_size = block_size*inodetable.i_block[0];
	return directory_block_size;
	
}


// Information regarding directory entry
read_directory_entries(int fd, int directory_block_size)
{
	struct ext2_dir_entry_2 directory_entry;
	int i = parts_length;
	int j = 0;
	int size_sec = 0;
	int size = 0;
	char file_name[NAMELEN+1];

	lseek(fd, directory_block_size, SEEK_SET);
	read(fd, &directory_entry, sizeof(directory_entry));
	// if directory path has multiple file names
	if (i > 0)
	{
		lseek(fd, directory_block_size, SEEK_SET);
		read(fd, &directory_entry, sizeof(directory_entry));
		printf("\n");

		while(i>j)
		{
			if(strcmp(dir_parts[j],directory_entry.name)==0)
			{
				struct ext2_inode inodetable;
				directory_block_size = calc_inode_offset(directory_entry.inode);
				lseek(fd, directory_block_size, SEEK_SET);
		 		read(fd, &inodetable, sizeof(inodetable));
		 		//printf("inode blcok 0 : %d\n", inodetable.i_block[0]);
		 		directory_block_size = block_size*inodetable.i_block[0];
		 	 	//printf("Directory block_size : %d\n", directory_block_size );
		 	// 	printf("\n blocks count 		: %hu\t", inodetable.i_blocks);
		 		i--;
		 		j++;
		 		exit;
			}
			else
			{
				if ((block_size - size) == directory_entry.rec_len)
				{
					exit;
				}
				else
				{

					size += directory_entry.rec_len ;
					directory_block_size += directory_entry.rec_len;

				}
			}

				lseek(fd, directory_block_size, SEEK_SET);
				read(fd, &directory_entry, sizeof(directory_entry));


		}	
	}
	calc_inode_metadata(fd,directory_entry.inode);
	if ( (inode_imode & S_IFMT) != S_IFDIR )
	{
		printf("The path given is not  a Directory path : %s\n\n", path_to_directory);
		exit(0);
	}
	//printf("\n\n   Inode_no\t Rec_length\t mod.time \t File_Name\n");
	printf("\n");
	printf("\tInode\tUID\tSize\tAccess Rights\tFile Name\tModified Time\n ");
	printf("======================================================================================\n");
	do
	{
			
			lseek(fd, directory_block_size, SEEK_SET);
			read(fd, &directory_entry, sizeof(directory_entry));
			int total_bytes = 8 ;
			int name_length_check = directory_entry.name_len ;
			while((name_length_check % 4) != 0)
			{
				name_length_check++;
			}
			total_bytes += name_length_check;
			char file_name[NAMELEN+1];
			memcpy(file_name, directory_entry.name, directory_entry.name_len);
			file_name[directory_entry.name_len] = 0;
			//printf("inode Number : %d\n", directory_entry.inode );
			calc_inode_metadata(fd,directory_entry.inode);
			//printf("inode_mtime : %u\n", inode_mtime);
			time_t file_mtime = inode_mtime;
			printf("\t%d\t%d\t%d\t%s\t%10s\t%s ",directory_entry.inode, inode_uid, inode_size, permissions, file_name, asctime(gmtime(&file_mtime)) );
			// checking for last entry in the directory
			
			if(command_option == 2)
			{
				if ((block_size - size_sec) == directory_entry.rec_len)
				{
					break;
				}
				else if (total_bytes != directory_entry.rec_len)
				{
					size_sec += total_bytes ;
					directory_block_size += total_bytes;
				}
			 	else
				{
					size_sec += directory_entry.rec_len ;
					directory_block_size += directory_entry.rec_len;
				}
			}else if ((block_size - size_sec) == directory_entry.rec_len)
			{
				break;
			}
			 else
			{
				size_sec += directory_entry.rec_len ;
				directory_block_size += directory_entry.rec_len;
			}
						
	} while((block_size - size_sec) != directory_entry.rec_len );
	printf("\n");
}



//calculate block address of the inode
calc_inode_offset(int inode)
{
	int group_no, inode_no_group, inode_blockno_reltogroup, inode_offset_inthatblock, inode_address;
	group_no = (inode-1)/ipg;
	inode_no_group = (inode-1)%ipg;
	inode_blockno_reltogroup = inode_no_group / ipb;
	inode_offset_inthatblock = inode_no_group % ipb;
	inode_address = (Inode_table_start + bpg * group_no) + inode_blockno_reltogroup;
	inode_address += (inode_offset_inthatblock * sizeof(struct ext2_inode));
	
	return inode_address;
}



calc_inode_metadata(int fd, int inode)
{
	struct ext2_inode inodetable;
	int block_address = calc_inode_offset(inode);
	lseek(fd, block_address, SEEK_SET);
	read(fd, &inodetable, sizeof(inodetable));

	inode_size	=	inodetable.i_size;
	inode_imode	=	inodetable.i_mode;
	inode_uid	=	inodetable.i_uid;
	inode_mtime		= 	inodetable.i_mtime;

	// Directory or regualr file
	if ( (inodetable.i_mode & S_IFMT) == S_IFDIR)
	{
		permissions[0] 	= 'd';
	}
	else
	{
		permissions[0]	= '-';
	}

	// User level permissions
	// Read
	if (inode_imode & S_IRUSR)
	{
		permissions[1]	= 'r';
	}
	else
	{
		permissions[1]	= '-';
	}
	// Write
	if (inode_imode & S_IWUSR)
	{
		permissions[2]	= 'w';
	}
	else
	{
		permissions[2]	= '-';
	}
	//Execute
	if (inode_imode & S_IXUSR)
	{
		permissions[3]	= 'x';
	}
	else
	{
		permissions[3]	= '-';
	}
	// Group level permissions
	// Read
	if (inode_imode & S_IRGRP)
	{
		permissions[4]	= 'r';
	}
	else
	{
		permissions[4]	= '-';
	}
	// Write
	if (inode_imode & S_IWGRP)
	{
		permissions[5]	= 'w';
	}
	else
	{
		permissions[5]	= '-';
	}
	//Execute
	if (inode_imode & S_IXGRP)
	{
		permissions[6]	= 'x';
	}
	else
	{
		permissions[6]	= '-';
	}
	// All other level permissions
	// Read
	if (inode_imode & S_IROTH)
	{
		permissions[7]	= 'r';
	}
	else
	{
		permissions[7]	= '-';
	}
	// Write
	if (inode_imode & S_IWOTH)
	{
		permissions[8]	= 'w';
	}
	else
	{
		permissions[8]	= '-';
	}
	//Execute
	if (inode_imode & S_IXOTH)
	{
		permissions[9]	= 'x';
	}
	else
	{
		permissions[9]	= '-';
	}
	
}
