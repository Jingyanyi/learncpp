#ifndef _LOCKLESS_HPP_
#define _LOCKLESS_HPP_

#include <iostream>
#include <memory>
#include <vector>

//               LT      T                   LH      H
// | _ | _ | _ | x | x | x | x | x | x | x | _ | _ | _ |
//               ^   ^                       ^   ^
//               |   |                       |   |
//               C1  C2                      P1  P2

template <typename T>
class Lockless
{
typedef volatile size_t atomic_t;

public:
	explicit Lockless(size_t sz):
		data_()
	{
		++ sz;  //head tail会空出一个空位
		size_t align_size = 1;
		if(sz < 2)
			abort();
			
		while(align_size < sz)
			align_size <<= 1;
			
		count_ = align_size;
		mask_  = align_size - 1;
		data_.resize(count_);
		
		head_ = least_head_ = 0;
		tail_ = least_tail_ = 0;
		
		std::cout << "count: " << count_ << ", mask: "  << mask_ << std::endl; 
			
	}
	
	void push(const T& item)
	{
		size_t head = head_;
		
		while(next(head_) == least_tail_) {
			::sched_yield();	
		}
		
		do{
			head = head_;
		} while( !__sync_bool_compare_and_swap(&head_, head, next(head)) );
		
		data_[head] = item;
				
		while( !__sync_bool_compare_and_swap(&least_head_, head, next(head)) ) {
			::sched_yield();
		} 
		
		return;
	}
	
	T pop()
	{
		T t;
		size_t tail = tail_;
		
		while(tail_ == least_head_) {
			::sched_yield();	
		}
		
		do{
			tail = tail_;
		} while( !__sync_bool_compare_and_swap(&tail_, tail, next(tail)) );
		
		t = data_[tail];
		
		while( !__sync_bool_compare_and_swap(&least_tail_, tail, next(tail)) ){
			::sched_yield();
		}
				
		return t;
	}

	~Lockless() {}

private:
	atomic_t next(const atomic_t& curr)
	{
		return ((curr + 1) & mask_);
	}
	
	std::vector<T> data_;
	size_t		   count_;
	size_t         mask_;
	
	atomic_t head_;
	atomic_t tail_;
	atomic_t least_head_;
	atomic_t least_tail_;
};

#endif  // _LOCKLESS_HPP_