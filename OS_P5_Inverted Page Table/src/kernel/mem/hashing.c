#include <device/io.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <device/console.h>
#include <proc/proc.h>
#include <interrupt.h>
#include <mem/palloc.h>
#include <ssulib.h>
#include <mem/hashing.h>

int check = 0;

uint32_t F_IDX(uint32_t addr, uint32_t capacity) {
	return addr % ((capacity / 2) - 1);
}

uint32_t S_IDX(uint32_t addr, uint32_t capacity) {
	return (addr * 7) % ((capacity / 2) - 1) + capacity / 2;
}

void init_hash_table(void)
{
	// TODO : OS_P5 assignment
	for(int i = 0; i < CAPACITY; i++){
		for(int j = 0; j < SLOT_NUM; j++){
			hash_table.top_buckets[i].token[j] = 0;
			hash_table.top_buckets[i].slot[j].key = -1;
			hash_table.top_buckets[i].slot[j].value = -1;

			if(i % 2 == 0){
				hash_table.bottom_buckets[i/2].token[j] = 0;
				hash_table.bottom_buckets[i/2].slot[j].key = -1;
				hash_table.bottom_buckets[i/2].slot[j].value = -1;
			}
		}
	}
}

void hash_insert(size_t page_idx, void *pages){

	uint32_t value = (uint32_t)VH_TO_RH((uint32_t)pages);
	int key = page_idx;
	uint32_t hash1_idx = F_IDX((uint32_t)pages, CAPACITY);
	uint32_t hash2_idx = S_IDX((uint32_t)pages, CAPACITY);
	int insert_check = false;

	for(int i = 0; i < SLOT_NUM; i++){
		if(hash_table.top_buckets[hash1_idx].token[i] == 0){	// top_level의 왼쪽 버킷 삽입
			hash_table.top_buckets[hash1_idx].slot[i].value = value;
			hash_table.top_buckets[hash1_idx].slot[i].key = key;
			hash_table.top_buckets[hash1_idx].token[i] = 1;
			insert_check = true;
			printk("hash value inserted in top level : idx : %d, key : %d, value : %x\n", hash1_idx, key, value);
			break;
		}

		else if(hash_table.top_buckets[hash2_idx].token[i] == 0){	// top_level의 오른쪽 버킷 삽입
			hash_table.top_buckets[hash2_idx].slot[i].value = value;
			hash_table.top_buckets[hash2_idx].slot[i].key = key;
			hash_table.top_buckets[hash2_idx].token[i] = 1;
			insert_check = true;
			printk("hash value inserted in top level : idx : %d, key : %d, value : %x\n", hash2_idx, key, value);
			break;
		}
	}

	if(!insert_check){	// top_level삽입 실패시 bottom_level삽입
		for(int i = 0; i < SLOT_NUM; i++){
			if(hash_table.bottom_buckets[hash1_idx/2].token[i] == 0){	// bottom_level의 왼쪽 버킷 삽입
				hash_table.bottom_buckets[hash1_idx/2].slot[i].value = value;
				hash_table.bottom_buckets[hash1_idx/2].slot[i].key = key;
				hash_table.bottom_buckets[hash1_idx/2].token[i] = 1;
				insert_check = true;
				printk("hash value inserted in bottom level : idx : %d, key : %d, value : %x\n", hash1_idx/2, key, value);
				break;
			}

			else if(hash_table.bottom_buckets[hash2_idx/2].token[i] == 0){	// bottom_level의 오른쪽 버킷 삽입
				hash_table.bottom_buckets[hash2_idx/2].slot[i].value = value;
				hash_table.bottom_buckets[hash2_idx/2].slot[i].key = key;
				hash_table.bottom_buckets[hash2_idx/2].token[i] = 1;
				insert_check = true;
				printk("hash value inserted in bottom level : idx : %d, key : %d, value : %x\n", hash2_idx/2, key, value);
				break;
			}

		}
	}

	uint32_t tmpvalue, tmphash1_idx, tmphash2_idx;

	if(!insert_check){	// top_level, bottom_level삽입 실패 시 인덱스 이동
		for(int i = 0; i < SLOT_NUM; i++){
			tmpvalue = (uint32_t)RH_TO_VH(hash_table.top_buckets[hash1_idx].slot[i].value);	// 해쉬를 위한 가상주소 변환
			tmphash1_idx = F_IDX(tmpvalue, CAPACITY);
			tmphash2_idx = S_IDX(tmpvalue, CAPACITY);

			for(int j = 0; j < SLOT_NUM; j++){	// 왼쪽 버킷 탐색
				if(hash_table.top_buckets[tmphash2_idx].token[j] == 0){	// top_level의 오른쪽 버킷이 비어 있을 시
					hash_table.top_buckets[tmphash2_idx].slot[j] = hash_table.top_buckets[hash1_idx].slot[i];
					hash_table.top_buckets[tmphash2_idx].token[j] = 1;
					hash_table.top_buckets[hash1_idx].slot[i].key = key;
					hash_table.top_buckets[hash1_idx].slot[i].value = value;
					insert_check = true;
					printk("moving hash value in top level %d to %d\n", hash1_idx, tmphash2_idx);
					printk("hash value inserted in top level : idx : %d, key : %d, value : %x\n", hash1_idx, key, j, value);
					break;
				}
			}

			if(insert_check){
				break;
			}

			tmpvalue = (uint32_t)RH_TO_VH(hash_table.top_buckets[hash2_idx].slot[i].value);
			tmphash1_idx = F_IDX(tmpvalue, CAPACITY);
			tmphash2_idx = S_IDX(tmpvalue, CAPACITY);

			for(int j = 0; j < SLOT_NUM; j++){	// 오른쪽 버킷 탐색
				if(hash_table.top_buckets[tmphash1_idx].token[j] == 0){	// top_level의 왼쪽 버킷이 비어 있을 시
					hash_table.top_buckets[tmphash1_idx].slot[j] = hash_table.top_buckets[hash2_idx].slot[i];
					hash_table.top_buckets[tmphash1_idx].token[j] = 1;
					hash_table.top_buckets[hash2_idx].slot[i].key = key;
					hash_table.top_buckets[hash2_idx].slot[i].value = value;
					insert_check = true;
					printk("moving hash value in top level %d to %d\n", hash2_idx, tmphash1_idx);
					printk("hash value inserted in top level : idx : %d, key : %d, value : %x\n", hash2_idx, key, value);
					break;

				}
			}

			if(insert_check){
				break;
			}
		}

		if(!insert_check){
			for(int i = 0; i < SLOT_NUM; i++){
				tmpvalue = (uint32_t)RH_TO_VH(hash_table.bottom_buckets[hash1_idx/2].slot[i].value);	// 해쉬를 위한 가상주소 변환

				tmphash1_idx = F_IDX(tmpvalue, CAPACITY);
				tmphash2_idx = S_IDX(tmpvalue, CAPACITY);

				for(int j = 0; j < SLOT_NUM; j++){	// 왼쪽 버킷 탐색
					if(hash_table.bottom_buckets[tmphash2_idx/2].token[j] == 0){	// bottom_level의 오른쪽 버킷이 비어 있을 시
						hash_table.bottom_buckets[tmphash2_idx/2].slot[j] = hash_table.bottom_buckets[hash1_idx/2].slot[i];
						hash_table.bottom_buckets[tmphash2_idx/2].token[j] = 1;
						hash_table.bottom_buckets[hash1_idx/2].slot[i].key = key;
						hash_table.bottom_buckets[hash1_idx/2].slot[i].value = value;
						insert_check = true;
						printk("moving hash value in bottom level %d to %d\n", hash1_idx/2, tmphash2_idx/2);
						printk("hash value inserted in bottom level : idx : %d, key : %d, value : %x\n", hash1_idx/2, key, value);
						break;
					}
				}

				if(insert_check){
					break;
				}

				tmpvalue = (uint32_t)RH_TO_VH(hash_table.bottom_buckets[hash2_idx/2].slot[i].value);
				tmphash1_idx = F_IDX(tmpvalue, CAPACITY);
				tmphash2_idx = S_IDX(tmpvalue, CAPACITY);

				for(int j = 0; j < SLOT_NUM; j++){	// 오른쪽 버킷 탐색
					if(hash_table.bottom_buckets[tmphash1_idx/2].token[j] == 0){	// bottom_level의 왼쪽 버킷이 비어 있을 시
						hash_table.bottom_buckets[tmphash1_idx/2].slot[j] = hash_table.bottom_buckets[hash2_idx/2].slot[i];
						hash_table.bottom_buckets[tmphash1_idx/2].token[j] = 1;
						hash_table.bottom_buckets[hash2_idx/2].slot[i].key = key;
						hash_table.bottom_buckets[hash2_idx/2].slot[i].value = value;
						insert_check = true;
						printk("moving hash value in bottom level %d to %d\n", hash2_idx/2, tmphash1_idx/2);
						printk("hash value inserted in bottom level : idx : %d, key : %d, value : %x\n", hash2_idx/2, key, value);
						break;

					}
				}

				if(insert_check){
					break;
				}
			}
		}
	}

	if(!insert_check){		// Resizing
		printk("Resizing\n");
	}
}

void hash_delete(uint32_t *pages, size_t page_idx){

	uint32_t hash1_idx = F_IDX((uint32_t)pages, CAPACITY);
	uint32_t hash2_idx = S_IDX((uint32_t)pages, CAPACITY);
	int delete_check = false;
	int key1, key2;
	uint32_t value1, value2;
	uint32_t value = (uint32_t)VH_TO_RH(pages);

	for(int i = 0; i < SLOT_NUM; i++){
		key1 = hash_table.top_buckets[hash1_idx].slot[i].key;
		value1 = hash_table.top_buckets[hash1_idx].slot[i].value;
		key2 = hash_table.top_buckets[hash2_idx].slot[i].key;
		value2 = hash_table.top_buckets[hash2_idx].slot[i].value;

		if(hash_table.top_buckets[hash1_idx].token[i] == 1 && page_idx == key1){
			hash_table.top_buckets[hash1_idx].token[i] = 0;	
			printk("hash value deleted : idx : %d, key : %d, value : %x\n", hash1_idx, key1, value1);
			delete_check = true;
			break;

		}

		else if(hash_table.top_buckets[hash2_idx].token[i] == 1 && page_idx == key2){
			hash_table.top_buckets[hash2_idx].token[i] = 0;
			printk("hash value deleted : idx : %d, key : %d, value : %x\n", hash2_idx, key2, value2);
			delete_check = true;
			break;
		}
	}

	if(!delete_check){	// top_level에 없을 시 bottom_level 검색해서 삭제
		for(int i = 0; i < SLOT_NUM; i++){

			key1 = hash_table.bottom_buckets[hash1_idx/2].slot[i].key;
			value1 = hash_table.bottom_buckets[hash1_idx/2].slot[i].value;
			key2 = hash_table.bottom_buckets[hash2_idx/2].slot[i].key;
			value2 = hash_table.bottom_buckets[hash2_idx/2].slot[i].value;

			if(hash_table.bottom_buckets[hash1_idx/2].token[i] == 1 && page_idx == key1){
				hash_table.bottom_buckets[hash1_idx/2].token[i] = 0;	
				printk("hash value deleted : idx : %d, key : %d, value : %x\n", hash1_idx/2, key1, value1);
				delete_check = true;
				break;
			}

			else if(hash_table.bottom_buckets[hash2_idx/2].token[i] == 1 && page_idx == key2){
				hash_table.bottom_buckets[hash2_idx/2].token[i] = 0;
				printk("hash value deleted : idx : %d, key : %d, value : %x\n", hash2_idx/2, key2, value2);
				delete_check = true;
				break;
			}
		}
	}
}
