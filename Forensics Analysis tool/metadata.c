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

#define EXT2_S_IFREG    0x8000    /*regular file*/

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
int inode_imode;
unsigned int inode_size;
int command_option;
char permissions[10];
unsigned int directory_inode;
unsigned int file_inode;

// Usage:  "./display_dir file_name directory_path"
int main(int argc, char **argv) 
{
	const char delim[2] = "/";
  	// open the ext2 file 
	  if ((argc > 2) && (argc < 5)) 
	  {
		    // Do nothing
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
		if(strcmp(argv[3], "-a") == 0)
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
	int reg_file_flag = 0;
	int inode_temp;
	int size = 0;
	char file_name[NAMELEN+1];


	// if directory path has multiple file names
	if (i > 0)
	{
		// lseek(fd, directory_block_size, SEEK_SET);
		// read(fd, &directory_entry, sizeof(directory_entry));
		printf("\n");

		while(i>j)
		{
			// printf("in while loop\n");
			lseek(fd, directory_block_size, SEEK_SET);
			read(fd, &directory_entry, sizeof(directory_entry));
			int total_bytes = 8 ;
			int name_length_check = directory_entry.name_len ;
			while((name_length_check % 4) != 0)
			{
				name_length_check++;
			}
			total_bytes += name_length_check;
			struct ext2_inode inodetable;
			int inode_temp;
			// printf("dir_parts  :%s\n", dir_parts[j]);
			// printf("Directory name : %s\n", directory_entry.name);
			memcpy(file_name, directory_entry.name, directory_entry.name_len);
			file_name[directory_entry.name_len] = 0;
			// printf("file name : %s\n", file_name );
			if(strcmp(dir_parts[j],file_name)==0)
			{	
				//printf("inside\n");
				directory_block_size = calc_inode_offset(directory_entry.inode);
				inode_temp = directory_entry.inode;
				//printf("inode_temp : %d\n", inode_temp );
				lseek(fd, directory_block_size, SEEK_SET);
		 		read(fd, &inodetable, sizeof(inodetable));
		 		if ( (inodetable.i_mode & S_IFMT) == S_IFDIR)
				{	
					directory_inode = inode_temp;
					directory_block_size = block_size*inodetable.i_block[0];
					//printf("directory_inode out : %d\n", directory_inode);
					
				}
				else
				{
					reg_file_flag = 3;	
					file_inode = inode_temp;	 
					// printf("file_inode out:%d\n", file_inode);		
				}
				i--;
				j++;
				break;	
			}
			
			if ((block_size - size) == directory_entry.rec_len)
			{
				break;
			}else if (total_bytes != directory_entry.rec_len)
					{
						size += total_bytes ;
						directory_block_size += total_bytes;
					}
				 	else
					{
						size += directory_entry.rec_len ;
						directory_block_size += directory_entry.rec_len;
					}
					lseek(fd, directory_block_size, SEEK_SET);
					read(fd, &directory_entry, sizeof(directory_entry));
		}	
		
	}
	else		// for directory path "/"
	{
		directory_inode = 2;
		print_directory_metadata(fd,directory_block_size);
	}
	calc_inode_metadata(fd, directory_entry.inode);
	// for directory path "/xxxx"
	if ( (inode_imode & S_IFMT) == S_IFDIR )
	{
		print_directory_metadata(fd, directory_block_size,directory_entry.inode);
	}
	if ( reg_file_flag == 3 )
	{
		print_inode_metadata(fd);
	}
	
	printf("\n");
}


// Prints directory info.
print_directory_metadata(int fd, int directory_block_size)
{
	int size_sec = 0;
	struct ext2_dir_entry_2 directory_entry;
	struct ext2_inode inodetable;
	printf("\n");
	printf("\t\tMeta-Data information of '%s' directory\n", path_to_directory );
	printf("\t\t--------------------------------------------------\n\n");
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
					exit;
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
					exit;
				}
				else
				{
					size_sec += directory_entry.rec_len ;
					directory_block_size += directory_entry.rec_len;
				}
						
		} while((block_size - size_sec) != directory_entry.rec_len );

		
		int directory_inode_address = calc_inode_offset(directory_inode);
		
		// Directory inode information
		lseek(fd, directory_inode_address, SEEK_SET);
		read(fd, &inodetable, sizeof(inodetable));
		time_t i_mtime = inodetable.i_mtime;
		time_t i_ctime = inodetable.i_ctime;
		time_t i_atime = inodetable.i_atime;
		time_t i_dtime = inodetable.i_dtime;
		__u32 single_indirect_block;

	printf("\n\n\tInode Information of '%s' Directory\n", path_to_directory);
	printf("=========================================================================\n");
	printf("\tInode Number 			: \t%d\n", directory_inode);
	printf("\tUID 				: \t%d\n",inodetable.i_uid );
	printf("\tFile Size 			: \t%d\n", inodetable.i_size );
	printf("\tNo of Blocks 			: \t%d\n", inodetable.i_blocks);
	printf("\tAccess Rights 			: \t%s\n", permissions );
	printf("\tLower 16 bits of Group ID is 	: \t%d\n", inodetable.i_gid);
	printf("\tLinks count 			: \t%d\n", inodetable.i_links_count);
	printf("\tCreation Time 			: \t%s",  asctime(gmtime(&i_ctime)));
	printf("\tLast Modified Time 		: \t%s",  asctime(gmtime(&i_mtime)));
	printf("\tLast Access Time 		: \t%s",  asctime(gmtime(&i_atime)));
	printf("\tDeletion Time 			: \t%s",  asctime(gmtime(&i_dtime)));
	
	printf("\tDirect Blocks			: \t ");
	int i =0;
	for (i = 0; i < 12; i++)
	{
		if (inodetable.i_block[i] != 0)
		{
			printf("%d", inodetable.i_block[i]);
			printf("\n\t\t\t\t\t\t");
		}
	}
	if (inodetable.i_block[12] != 0)
	{
		int single_indirect_block_address = (block_size * inodetable.i_block[12]);
		lseek(fd, single_indirect_block_address, SEEK_SET);
		read(fd, &single_indirect_block, sizeof(single_indirect_block));
		printf("\t  Single indirect block 			: \t");
		while(single_indirect_block != 0)
		{
			printf("%d", single_indirect_block);
			single_indirect_block_address += 4;
			lseek(fd, single_indirect_block_address, SEEK_SET);
			read(fd, &single_indirect_block, sizeof(single_indirect_block));
			printf("  ");
		}
		printf("\n\n");
	}
	else
	{
		printf(" No Single indirect blocks\n");

	}
}

print_inode_metadata(int fd)
{
	struct ext2_inode inodetable;
	int inode_block_address = calc_inode_offset(file_inode);
	lseek(fd, inode_block_address, SEEK_SET);
	read(fd, &inodetable, sizeof(inodetable));
	time_t i_mtime = inodetable.i_mtime;
	time_t i_ctime = inodetable.i_ctime;
	time_t i_atime = inodetable.i_atime;
	time_t i_dtime = inodetable.i_dtime;
	__u32 single_indirect_block;

	// Displaying the values from the inodes table block
	//printf("%d\t%d\t%d\t%d\t%s\t%s\t%s\t\n", inode, inodetable.i_uid, inodetable.i_size, inodetable.i_blocks, permissions, asctime(gmtime(&i_ctime)), asctime(gmtime(&i_mtime))  );
	printf("\n\tMeta-Data Information of '%s' file\n", path_to_directory);
	printf("============================================================================\n");
	printf("\tInode Number 			: \t%d\n", file_inode);
	printf("\tUID 				: \t%d\n",inodetable.i_uid );
	printf("\tFile Size 			: \t%d\n", inodetable.i_size );
	printf("\tNo of Blocks 			: \t%d\n", inodetable.i_blocks);
	printf("\tAccess Rights 			: \t%s\n", permissions );
	printf("\tLower 16 bits of Group ID is 	: \t%d\n", inodetable.i_gid);
	printf("\tLinks count 			: \t%d\n", inodetable.i_links_count);
	printf("\tCreation Time 			: \t%s",  asctime(gmtime(&i_ctime)));
	printf("\tLast Modified Time 		: \t%s",  asctime(gmtime(&i_mtime)));
	printf("\tLast Access Time 		: \t%s",  asctime(gmtime(&i_atime)));
	printf("\tDeletion Time 			: \t%s",  asctime(gmtime(&i_dtime)));
	
	printf("\tDirect Blocks			: \t ");
	int i =0;
	for (i = 0; i < 12; i++)
	{
		if (inodetable.i_block[i] != 0)
		{
			printf("%d", inodetable.i_block[i]);
			printf(" ");
		}
	}
	// Single indirect blocks
	if (inodetable.i_block[12] != 0)
	{
		int single_indirect_block_address = (block_size * inodetable.i_block[12]);
		lseek(fd, single_indirect_block_address, SEEK_SET);
		read(fd, &single_indirect_block, sizeof(single_indirect_block));
		printf("\t\t\t  Single indirect block 			: \t");
		while(single_indirect_block != 0)
		{
			printf("%d", single_indirect_block);
			single_indirect_block_address += 4;
			lseek(fd, single_indirect_block_address, SEEK_SET);
			read(fd, &single_indirect_block, sizeof(single_indirect_block));
			printf("  ");
		}
		printf("\n\n");
	}
	else
	{
		printf("\n\tNo Single indirect blocks\n");

	}
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


// Calculate metadata information of a particular inode.
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
