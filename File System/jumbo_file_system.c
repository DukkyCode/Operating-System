#include "jumbo_file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C does not have a bool type, so I created one that you can use
typedef char bool_t;
#define TRUE 1
#define FALSE 0


static block_num_t current_dir;

// optional helper function you can implement to tell you if a block is a dir node or an inode
static bool_t is_dir(block_num_t block_num) {
  char buffer[BLOCK_SIZE];

  //Read the block and transfer it to the buffer
  read_block(block_num, buffer);

  struct block *blk = (struct block *) buffer;

  if((blk->is_dir) == 0){
	return TRUE;
  }
  else{
	return FALSE;
  }

}


/* jfs_mount
 *   prepares the DISK file on the _real_ file system to have file system
 *   blocks read and written to it.  The application _must_ call this function
 *   exactly once before calling any other jfs_* functions.  If your code
 *   requires any additional one-time initialization before any other jfs_*
 *   functions are called, you can add it here.
 * filename - the name of the DISK file on the _real_ file system
 * returns 0 on success or -1 on error; errors should only occur due to
 *   errors in the underlying disk syscalls.
 */
int jfs_mount(const char* filename) {
  int ret = bfs_mount(filename);
  current_dir = 1;
  return ret;
}


/* jfs_mkdir
 *   creates a new subdirectory in the current directory
 * directory_name - name of the new subdirectory
 * returns 0 on success or one of the following error codes on failure:
 *   E_EXISTS, E_MAX_NAME_LENGTH, E_MAX_DIR_ENTRIES, E_DISK_FULL
 */
int jfs_mkdir(const char* directory_name) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;
  int dirname_len = strlen(directory_name);
  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  /////////////////Preliminary Checks/////////////////
  //Check if the directory exist
  for(i = 0; i < no_entries; i++){
        //Compare between the two strings
	if(strcmp((blk->contents).dirnode.entries[i].name, directory_name) == 0){
		//Return the flag
		return E_EXISTS;
	}
  }

  //Check if exceeds the maximum name lengths
  if(dirname_len > MAX_NAME_LENGTH){
	//Return the flag
	return E_MAX_NAME_LENGTH;
  }

  //Check if exceeds the maximum number of total files and subdirectories that can be a directory
  if(no_entries == MAX_DIR_ENTRIES){
	return E_MAX_DIR_ENTRIES;
  }

  //Check if Disk is Full
  block_num_t new_block = allocate_block();
  if(new_block == 0){
	return E_DISK_FULL;
  }

  //Making the directory
  //Change the data of current directory
  (blk->contents).dirnode.entries[no_entries].block_num = new_block;

  //Assign the name of the directory to struct block
  for(i = 0; i < dirname_len + 1; i++){
	(blk->contents).dirnode.entries[no_entries].name[i] = directory_name[i];
  }

  //Increment the number of entries
  (blk->contents).dirnode.num_entries++;

  //Write back to the current dir
  write_block(current_dir, (void *) blk);

  //Modify the new block value
  //Allocating Buffer and Initialization
  char buffer_newblk[BLOCK_SIZE];
  read_block(new_block, buffer_newblk);

  struct block *newdir_blk = (struct block *) buffer_newblk;

  //Setting new value
  (newdir_blk->is_dir) = 0;
  (newdir_blk->contents).dirnode.num_entries = 0;

  //Write back
  write_block(new_block, (void *) newdir_blk);

  return E_SUCCESS;
}


/* jfs_chdir
 *   changes the current directory to the specified subdirectory, or changes
 *   the current directory to the root directory if the directory_name is NULL
 * directory_name - name of the subdirectory to make the current
 *   directory; if directory_name is NULL then the current directory
 *   should be made the root directory instead
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_NOT_DIR
 */
int jfs_chdir(const char* directory_name){
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  //Go back to root if there's no argument
  if(directory_name == NULL){
        current_dir = 1;
        return E_SUCCESS;
  }
  //If there is argument
  for(i = 0; i < no_entries; i++){
	//Find the name of the arguments in the list
	if(strcmp((blk->contents).dirnode.entries[i].name, directory_name) == 0){
		//If matches, check if it is a directory
		block_num_t temp = (blk->contents).dirnode.entries[i].block_num;
		if(is_dir(temp)){
			current_dir = temp;
			return E_SUCCESS;
		}
		else{
			return E_NOT_DIR;
		}
	}
  }

  return E_NOT_EXISTS;
}

/* jfs_ls
 *   finds the names of all the files and directories in the current directory
 *   and writes the directory names to the directories argument and the file
 *   names to the files argument
 * directories - array of strings; the function will set the strings in the
 *   array, followed by a NULL pointer after the last valid string; the strings
 *   should be malloced and the caller will free them
 * file - array of strings; the function will set the strings in the
 *   array, followed by a NULL pointer after the last valid string; the strings
 *   should be malloced and the caller will free them
 * returns 0 on success or one of the following error codes on failure:
 *   (this function should always succeed)
 */
int jfs_ls(char* directories[MAX_DIR_ENTRIES+1], char* files[MAX_DIR_ENTRIES+1]) {
  //Intialization
  char buffer[BLOCK_SIZE];
  int i = 0;
  int dir_count = 0;
  int file_count = 0;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  //Putting the name into the array
  for(i = 0; i < no_entries; i++){
	//Get the length of the block name and malloc
	int len = strlen((blk->contents).dirnode.entries[i].name);
	char *str_buffer = (char *) malloc(len + 1);
	//Copy the string into the string_buffer
	strncpy(str_buffer, (blk->contents).dirnode.entries[i].name, len + 1);

	if(is_dir((blk->contents).dirnode.entries[i].block_num)){
		directories[dir_count] = str_buffer;
		dir_count++;
	}
	else{
		files[file_count] = str_buffer;
		file_count++;
	}
  }

  //The last end valid of the string is followed by a NULL pointer
  directories[dir_count] = NULL;
  files[file_count] = NULL;

  //free(temp_string);
  return E_SUCCESS;
}


/* jfs_rmdir
 *   removes the specified subdirectory of the current directory
 * directory_name - name of the subdirectory to remove
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_NOT_DIR, E_NOT_EMPTY
 */
int jfs_rmdir(const char* directory_name) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  for(i = 0; i < no_entries; i++){
	//Scan for the argument name
        if(strcmp((blk->contents).dirnode.entries[i].name, directory_name) == 0){
		block_num_t temp = (blk->contents).dirnode.entries[i].block_num;
		//If matches, check if it is a directory
		if(is_dir((blk->contents).dirnode.entries[i].block_num)){
			//Initialize to get the info from the block
			char rmdir_buffer[BLOCK_SIZE];
			read_block(temp, rmdir_buffer);
			struct block *rmdir_blk = (struct block *) rmdir_buffer;

			//Check if the block have any entries
			if((rmdir_blk->contents).dirnode.num_entries != 0){
				return E_NOT_EMPTY;
			}

			//Remove the directory operation
			//Shift the directories to the left
			for(int j = i; j < no_entries; j++){
				(blk->contents).dirnode.entries[j] = (blk->contents).dirnode.entries[j + 1];
			}
			//Decrement the entry
			(blk->contents).dirnode.num_entries--;
			//Write back and release
			write_block(current_dir, (void *) blk);
			release_block(temp);
			return E_SUCCESS;

		}
		//This is not a directory
		else{
			return E_NOT_DIR;
		}

	}

  }
  return E_NOT_EXISTS;
}


/* jfs_creat
 *   creates a new, empty file with the specified name
 * file_name - name to give the new file
 * returns 0 on success or one of the following error codes on failure:
 *   E_EXISTS, E_MAX_NAME_LENGTH, E_MAX_DIR_ENTRIES, E_DISK_FULL
 */
int jfs_creat(const char* file_name) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;
  int file_len = strlen(file_name);

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  /////////////////Preliminary Checks/////////////////
  //Check if the directory exist
  for(i = 0; i < no_entries; i++){
        //Compare between the two strings
        if(strcmp((blk->contents).dirnode.entries[i].name, file_name) == 0){
                //Return the flag
                return E_EXISTS;
        }
  }

  //Check if exceeds the maximum name lengths
  if(file_len > MAX_NAME_LENGTH){
        //Return the flag
        return E_MAX_NAME_LENGTH;
  }

  //Check if exceeds the maximum number of total files and subdirectories that can be a directory
  if(no_entries == MAX_DIR_ENTRIES){
        return E_MAX_DIR_ENTRIES;
  }

  //Check if Disk is Full
  block_num_t new_block = allocate_block();
  if(new_block == 0){
        return E_DISK_FULL;
  }

  //Making the directory
  //Change the data of current directory
  (blk->contents).dirnode.entries[no_entries].block_num = new_block;

  //Assign the name of the directory to struct block
  for(i = 0; i < file_len + 1; i++){
        (blk->contents).dirnode.entries[no_entries].name[i] = file_name[i];
  }

  //Increment the number of entries
  (blk->contents).dirnode.num_entries++;

  //Write back to the current dir
  write_block(current_dir, (void *) blk);

  //Modify the new block value
  //Allocating Buffer and Initialization
  char buffer_newblk[BLOCK_SIZE];
  read_block(new_block, buffer_newblk);

  struct block *newfile_blk = (struct block *) buffer_newblk;

  //Setting new value
  (newfile_blk->is_dir) = 1;
  (newfile_blk->contents).inode.file_size = 0;

  //Write back
  write_block(new_block, (void *) newfile_blk);

  return E_SUCCESS;
}


/* jfs_remove
 *   deletes the specified file and all its data (note that this cannot delete
 *   directories; use rmdir instead to remove directories)
 * file_name - name of the file to remove
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_IS_DIR
 */
int jfs_remove(const char* file_name) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  for(i = 0; i < no_entries; i++){
        //Scan for the argument name
        if(strcmp((blk->contents).dirnode.entries[i].name, file_name) == 0){
                block_num_t temp = (blk->contents).dirnode.entries[i].block_num;
                //If matches, check if it is a file or directory
                if(!is_dir((blk->contents).dirnode.entries[i].block_num)){
                        //Initialize to get the info from the block
                        char rmfile_buffer[BLOCK_SIZE];
                        read_block(temp, rmfile_buffer);
                        struct block *rmfile_blk = (struct block *) rmfile_buffer;

                        //Remove the File operation
                        //Shift the file to the left
                        for(int j = i; j < no_entries; j++){
                                (blk->contents).dirnode.entries[j] = (blk->contents).dirnode.entries[j + 1];
                        }
                        //Decrement the entry and write back
                        (blk->contents).dirnode.num_entries--;
			write_block(current_dir, (void *) blk);

			//Operatation to release the block
			uint32_t file_size = (rmfile_blk->contents).inode.file_size;

			int file_totalblk = file_size/ BLOCK_SIZE;
			//If it is not a multiple of block size, increment 1 to store the remanining bytes
			if(file_size % BLOCK_SIZE != 0){
				file_totalblk++;
			}
			//Release all the data blocks
			for(int j = 0; j < file_totalblk; j++){
				release_block((rmfile_blk->contents).inode.data_blocks[j]);
			}
			//Release the file block
			release_block(temp);
			return E_SUCCESS;

                }
                //This is a directory
                else{
                        return E_IS_DIR;
                }

        }

  }

  return E_NOT_EXISTS;
}


/* jfs_stat
 *   returns the file or directory stats (see struct stat for details)
 * name - name of the file or directory to inspect
 * buf  - pointer to a struct stat (already allocated by the caller) where the
 *   stats will be written
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS
 */
int jfs_stat(const char* name, struct stats* buf) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  for(i = 0; i < no_entries; i++){
        //Scan for the argument name
        if(strcmp((blk->contents).dirnode.entries[i].name, name) == 0){
		//Get the data from the block
		block_num_t temp = (blk->contents).dirnode.entries[i].block_num;
		char target_buffer[BLOCK_SIZE];
		//Get the struct from the argument buffer
		read_block(temp, target_buffer);
		struct block *target_blk = (struct block *) target_buffer;

		//Transfer the information to the buf
		strncpy(buf->name, name, strlen(name) + 1);
		buf->block_num = temp;

		//Check if this is a directory
		if(is_dir(temp)){
			buf->is_dir = 0;
		}
		//If this a file
		else{
			buf->is_dir = 1;
			//Calculate the file size
			uint32_t file_size = (target_blk->contents).inode.file_size;
                        int file_totalblk = file_size/ BLOCK_SIZE;

			//If it is not a multiple of block size, increment 1 to store the remanining bytes
                        if(file_size % BLOCK_SIZE != 0){
                                file_totalblk++;
                        }

			buf->file_size = file_size;
			buf->num_data_blocks = file_totalblk;
		}

		return E_SUCCESS;

	}
  }

  return E_NOT_EXISTS;
}


/* jfs_write
 *   appends the data in the buffer to the end of the specified file
 * file_name - name of the file to append data to
 * buf - buffer containing the data to be written (note that the data could be
 *   binary, not text, and even if it is text should not be assumed to be null
 *   terminated)
 * count - number of bytes in buf (write exactly this many)
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_IS_DIR, E_MAX_FILE_SIZE, E_DISK_FULL
 */
int jfs_write(const char* file_name, const void* buf, unsigned short count) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  //Main loop
  for(i = 0; i < no_entries; i++){
        //Scan for the argument name
        if(strcmp((blk->contents).dirnode.entries[i].name, file_name) == 0){
                //Get the data from the block
                block_num_t temp = (blk->contents).dirnode.entries[i].block_num;
		if(!is_dir(temp)){
			char target_buffer[BLOCK_SIZE];
			//Get the struct of the target block
			read_block(temp, target_buffer);
			struct block *target_blk = (struct block *) target_buffer;

                        //Calculate the file size
                        uint32_t file_size = (target_blk->contents).inode.file_size;
                        uint32_t filenew_size = file_size + count;

			//Check if it exceeds max file size
			if(filenew_size > MAX_FILE_SIZE){
				return E_MAX_FILE_SIZE;
			}

			//Caculate initial data block
                        int file_totalblk = file_size/ BLOCK_SIZE;

                        //If it is not a multiple of block size, increment 1 to store the remanining bytes
                        if(file_size % BLOCK_SIZE != 0){
                                file_totalblk++;
                        }

			//Caculate the new file blocks
                        int filenew_totalblk = filenew_size/ BLOCK_SIZE;

                        //If it is not a multiple of block size, increment 1 to store the remanining bytes
                        if(filenew_size % BLOCK_SIZE != 0){
                                filenew_totalblk++;
                        }

			//Calculate blocks that need to be added
			int blk_append = filenew_totalblk - file_totalblk;
			block_num_t filenew_blk[blk_append];

			//Allocate new blocks for the file
			for(int j = 0; j < blk_append; j++){
				filenew_blk[j] = allocate_block();
				//If the Disk is full, release all the block and return
				if(filenew_blk[j] == 0){
					while(j >= 0){
						release_block(filenew_blk[j]);
						j--;
					}
					return E_DISK_FULL;
				}
			}

			//Modify the information of the file
			(target_blk->contents).inode.file_size = filenew_size;
			//Put the new block_num_t into the file block
			for(int k = 0; k < blk_append; k++){
				(target_blk->contents).inode.data_blocks[k + file_totalblk] = filenew_blk[k];
			}
			//Write back the information
			write_block(temp, target_blk);

			/////Append Operation/////
			//Buffer
			char write_buffer0[BLOCK_SIZE * (blk_append + 1)];
			char write_buffer1[BLOCK_SIZE * blk_append];
			char data_buffer[BLOCK_SIZE];
			int flag = FALSE;
			//Value
			int offset = file_size % BLOCK_SIZE;

			//Initialize the 2 arrays to be empty
			memset(write_buffer0, -1, BLOCK_SIZE * (blk_append + 1));
			memset(write_buffer1, -1, BLOCK_SIZE * blk_append);

			//If the block is not full
			if(offset != 0){
				//Read the last data block
				read_block((target_blk->contents).inode.data_blocks[file_totalblk - 1], data_buffer);
				//Copy the last data buffer to the write buffer
				memcpy(write_buffer0, data_buffer, BLOCK_SIZE);
				//Copy the append data to write_buffer0
				memcpy(&write_buffer0[offset], buf, count);
				flag = FALSE;

				//Write back data to the data block
				//write_block((target_blk->contents).inode.data_blocks[file_totalblk - 1], write_buffer0);
				//printf("The offset != 0\n");
			}
			//If the block is full
			else{
				if(file_size == 0){
					//Write back data to the data block
					memcpy(write_buffer1, buf, count);
					flag = TRUE;
					//write_block((target_blk->contents).inode.data_blocks[file_totalblk], write_buffer1);
					//printf("File size is 0\n");
				}
				else{
					//Read the last data block
                                	read_block((target_blk->contents).inode.data_blocks[file_totalblk - 1], data_buffer);
					//Copy the last data buffer to the write buffer1
					memcpy(write_buffer0, data_buffer, count);
					//Copy the append data to write_buffer1
					memcpy(&write_buffer0[BLOCK_SIZE], buf, count);
					flag = FALSE;
                                	//Write back data to the data block
                                	//write_block((target_blk->contents).inode.data_blocks[file_totalblk - 1], write_buffer0);
					//printf("File size is not 0 and offset is 0\n");

				}
			}

			//Write Back operation
			if(flag){
				int last = file_totalblk;
				for(int j = 0; last < filenew_totalblk; j++){
					write_block((target_blk->contents).inode.data_blocks[last], &write_buffer1[BLOCK_SIZE * j]);
					last++;
				}
			}
			else{
				int last = file_totalblk - 1;
                                for(int j = 0; last < filenew_totalblk; j++){
                                        write_block((target_blk->contents).inode.data_blocks[last], &write_buffer0[BLOCK_SIZE * j]);
                                        last++;
                                }
			}


			return E_SUCCESS;
		}
		else{
			return E_IS_DIR;
		}
	}
  }
  return E_NOT_EXISTS;
}


/* jfs_read
 *   reads the specified file and copies its contents into the buffer, up to a
 *   maximum of *ptr_count bytes copied (but obviously no more than the file
 *   size, either)
 * file_name - name of the file to read
 * buf - buffer where the file data should be written
 * ptr_count - pointer to a count variable (allocated by the caller) that
 *   contains the size of buf when it's passed in, and will be modified to
 *   contain the number of bytes actually written to buf (e.g., if the file is
 *   smaller than the buffer) if this function is successful
 * returns 0 on success or one of the following error codes on failure:
 *   E_NOT_EXISTS, E_IS_DIR
 */
int jfs_read(const char* file_name, void* buf, unsigned short* ptr_count) {
  //Initialization
  char buffer[BLOCK_SIZE];
  int i;

  //Get the struct of the current block
  read_block(current_dir, buffer);
  struct block *blk = (struct block *) buffer;
  uint16_t no_entries = (blk->contents).dirnode.num_entries;

  //Main loop
  for(i = 0; i < no_entries; i++){
        //Scan for the argument name
        if(strcmp((blk->contents).dirnode.entries[i].name, file_name) == 0){
                //Get the data from the block
                block_num_t temp = (blk->contents).dirnode.entries[i].block_num;
                if(!is_dir(temp)){
			//Get the struct of the target block
			char target_buffer[BLOCK_SIZE];
			read_block(temp, target_buffer);
			struct block * target_blk = (struct block *) target_buffer;

                        //Calculate the file size
                        uint32_t file_size = (target_blk->contents).inode.file_size;

                        //Caculate Total Data block
                        int file_totalblk = file_size/ BLOCK_SIZE;

                        //If it is not a multiple of block size, increment 1 to store the remanining bytes
                        if(file_size % BLOCK_SIZE != 0){
                                file_totalblk++;
                        }
			//Get the pointer count for file size
			if(file_size < *ptr_count){
				*ptr_count = file_size;
			}
			int32_t read_size = (int32_t) *ptr_count;

			char data_buffer[BLOCK_SIZE * (file_totalblk + 1)];

			//For loop to read the blocks
			for(int i = 0; read_size > 0; i++){
				read_block((target_blk->contents).inode.data_blocks[i], &data_buffer[i*BLOCK_SIZE]);
				read_size -= BLOCK_SIZE;
			}

			//Copy it to the buffer
			memcpy(buf, data_buffer, *ptr_count);
		
			return E_SUCCESS;

		}
		else{
			return E_IS_DIR;
		}
	}
  }
  return E_NOT_EXISTS;
}


/* jfs_unmount
 *   makes the file system no longer accessible (unless it is mounted again).
 *   This should be called exactly once after all other jfs_* operations are
 *   complete; it is invalid to call any other jfs_* function (except
 *   jfs_mount) after this function complete.  Basically, this closes the DISK
 *   file on the _real_ file system.  If your code requires any clean up after
 *   all other jfs_* functions are done, you may add it here.
 * returns 0 on success or -1 on error; errors should only occur due to
 *   errors in the underlying disk syscalls.
 */
int jfs_unmount() {
  int ret = bfs_unmount();
  return ret;
}
