#ifndef _CPU_PROCESSING_H_
#define _CPU_PROCESSING_H_

#include <chrono>
#include <atomic>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include "tbb/tbb.h"
#include "KernelArgs.h"

using namespace std;
using namespace tbb;

#define BATCH_SIZE 256
#define NUM_THREADS 48

void filter_probe_CPU(
  struct filterArgsCPU fargs, struct probeArgsCPU pargs, struct offsetCPU out_off, int num_tuples,
  int* total, int start_offset = 0, short* segment_group = NULL) {


  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);


  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) { //make sure the grainsize is not too big (will result in segfault on temp[5][end-start] -> not enough stack memory)
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int temp[5][end-start];
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    unsigned int count = 0;

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        // int slot1 = 1, slot2 = 1, slot3 = 1; 
        int slot4 = 1;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        // if (fargs.filter_col1 != NULL) {
          // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
        // }

        // if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        //   slot1 = slot >> 32;
        // }

        // if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        //   slot2 = slot >> 32;
        // }

        // if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        //   slot3 = slot >> 32;
        // }

        // if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          slot4 = slot >> 32;
        // }


        temp[0][count] = lo_offset;
        // temp[1][count] = slot1-1;
        // temp[2][count] = slot2-1;
        // temp[3][count] = slot3-1;
        temp[4][count] = slot4-1;
        count++;

      }
    }

    for (int i = end_batch ; i < end; i++) {
        int hash;
        long long slot;
        // int slot1 = 1, slot2 = 1, slot3 = 1; 
        int slot4 = 1;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        // if (fargs.filter_col1 != NULL) {
          // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
        // }

        // if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        //   slot1 = slot >> 32;
        // }

        // if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        //   slot2 = slot >> 32;
        // }

        // if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        //   slot3 = slot >> 32;
        // }

        // if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          slot4 = slot >> 32;
        // }


        temp[0][count] = lo_offset;
        // temp[1][count] = slot1-1;
        // temp[2][count] = slot2-1;
        // temp[3][count] = slot3-1;
        temp[4][count] = slot4-1;
        count++;
    }

    int thread_off = __atomic_fetch_add(total, count, __ATOMIC_RELAXED);

    for (int i = 0; i < count; i++) {
      assert(out_off.h_lo_off != NULL);
      if (out_off.h_lo_off != NULL) out_off.h_lo_off[thread_off+i] = temp[0][i];
      // if (out_off.h_dim_off1 != NULL) out_off.h_dim_off1[thread_off+i] = temp[1][i];
      // if (out_off.h_dim_off2 != NULL) out_off.h_dim_off2[thread_off+i] = temp[2][i];
      // if (out_off.h_dim_off3 != NULL) out_off.h_dim_off3[thread_off+i] = temp[3][i];
      if (out_off.h_dim_off4 != NULL) out_off.h_dim_off4[thread_off+i] = temp[4][i];
    }

  }, simple_partitioner());
}

void filter_probe_CPU2(struct offsetCPU in_off, struct filterArgsCPU fargs, struct probeArgsCPU pargs,
  struct offsetCPU out_off, int num_tuples, int* total, int start_offset = 0) {


  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(out_off.h_lo_off != NULL);
  assert(in_off.h_lo_off != NULL);


  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) { //make sure the grainsize is not too big (will result in segfault on temp[5][end-start] -> not enough stack memory)
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int temp[5][end-start];
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    unsigned int count = 0;
    
    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        // int slot1 = 1, slot2 = 1, slot3 = 1;
        int slot4 = 1;
        int lo_offset;

        lo_offset = in_off.h_lo_off[start_offset + i];

        // if (fargs.filter_col1 != NULL) {
          // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
          // if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
        // }

        // if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        //   slot1 = slot >> 32;
        // } else if (in_off.h_dim_off1 != NULL) slot1 = in_off.h_dim_off1[start_offset + i] + 1;


        // if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        //   slot2 = slot >> 32;
        // } else if (in_off.h_dim_off2 != NULL) slot2 = in_off.h_dim_off2[start_offset + i] + 1;


        // if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        //   slot3 = slot >> 32;
        // } else if (in_off.h_dim_off3 != NULL) slot3 = in_off.h_dim_off3[start_offset + i] + 1;


        // if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          slot4 = slot >> 32;
        // } else if (in_off.h_dim_off4 != NULL) slot4 = in_off.h_dim_off4[start_offset + i] + 1;


        temp[0][count] = lo_offset;
        // temp[1][count] = slot1-1;
        // temp[2][count] = slot2-1;
        // temp[3][count] = slot3-1;
        temp[4][count] = slot4-1;
        count++;

      }
    }

    for (int i = end_batch ; i < end; i++) {
      int hash;
      long long slot;
      // int slot1 = 1, slot2 = 1, slot3 = 1;
      int slot4 = 1;
      int lo_offset;

      lo_offset = in_off.h_lo_off[start_offset + i];

      // if (fargs.filter_col1 != NULL) {
        // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
        // if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
      // }

      // if (fargs.filter_col2 != NULL) {
        // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
        if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
      // }

      // if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
      //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
      //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
      //   if (slot == 0) continue;
      //   slot1 = slot >> 32;
      // } else if (in_off.h_dim_off1 != NULL) slot1 = in_off.h_dim_off1[start_offset + i] + 1;


      // if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
      //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
      //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
      //   if (slot == 0) continue;
      //   slot2 = slot >> 32;
      // } else if (in_off.h_dim_off2 != NULL) slot2 = in_off.h_dim_off2[start_offset + i] + 1;


      // if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
      //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
      //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
      //   if (slot == 0) continue;
      //   slot3 = slot >> 32;
      // } else if (in_off.h_dim_off3 != NULL) slot3 = in_off.h_dim_off3[start_offset + i] + 1;


      // if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
        slot4 = slot >> 32;
      // } else if (in_off.h_dim_off4 != NULL) slot4 = in_off.h_dim_off4[start_offset + i] + 1;


      temp[0][count] = lo_offset;
      // temp[1][count] = slot1-1;
      // temp[2][count] = slot2-1;
      // temp[3][count] = slot3-1;
      temp[4][count] = slot4-1;
      count++;
    }

    int thread_off = __atomic_fetch_add(total, count, __ATOMIC_RELAXED);

    for (int i = 0; i < count; i++) {
      assert(out_off.h_lo_off != NULL);
      if (out_off.h_lo_off != NULL) out_off.h_lo_off[thread_off+i] = temp[0][i];
      // if (out_off.h_dim_off1 != NULL) out_off.h_dim_off1[thread_off+i] = temp[1][i];
      // if (out_off.h_dim_off2 != NULL) out_off.h_dim_off2[thread_off+i] = temp[2][i];
      // if (out_off.h_dim_off3 != NULL) out_off.h_dim_off3[thread_off+i] = temp[3][i];
      if (out_off.h_dim_off4 != NULL) out_off.h_dim_off4[thread_off+i] = temp[4][i];
    }

  }, simple_partitioner());
}

void probe_CPU(
  struct probeArgsCPU pargs, struct offsetCPU out_off, int num_tuples,
  int* total, int start_offset = 0, short* segment_group = NULL) {


  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);
  assert(out_off.h_lo_off != NULL);


  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) { //make sure the grainsize is not too big (will result in segfault on temp[5][end-start] -> not enough stack memory)
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    //printf("worker_index = %d %d\n", worker_index, end-start);
    unsigned int temp[5][end-start];
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    unsigned int count = 0;


    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;
    
    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
      int hash;
      long long slot;
      int slot1 = 1, slot2 = 1, slot3 = 1, slot4 = 1;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
        hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        if (slot == 0) continue;
        slot1 = slot >> 32;
      }

      if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
        hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        if (slot == 0) continue;
        slot2 = slot >> 32;
      }

      if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
        hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        if (slot == 0) continue;
        slot3 = slot >> 32;
      }

      if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
        slot4 = slot >> 32;
      }

      temp[0][count] = lo_offset;
      temp[1][count] = slot1-1;
      temp[2][count] = slot2-1;
      temp[3][count] = slot3-1;
      temp[4][count] = slot4-1;
      count++;

      }
    }

    for (int i = end_batch ; i < end; i++) {
      int hash;
      long long slot;
      int slot1 = 1, slot2 = 1, slot3 = 1, slot4 = 1;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
        hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        if (slot == 0) continue;
        slot1 = slot >> 32;
      }

      if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
        hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        if (slot == 0) continue;
        slot2 = slot >> 32;
      }

      if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
        hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        if (slot == 0) continue;
        slot3 = slot >> 32;
      }

      if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
        slot4 = slot >> 32;
      }

      temp[0][count] = lo_offset;
      temp[1][count] = slot1-1;
      temp[2][count] = slot2-1;
      temp[3][count] = slot3-1;
      temp[4][count] = slot4-1;
      count++;
    }

    int thread_off = __atomic_fetch_add(total, count, __ATOMIC_RELAXED);

    for (int i = 0; i < count; i++) {
      assert(out_off.h_lo_off != NULL);
      if (out_off.h_lo_off != NULL) out_off.h_lo_off[thread_off+i] = temp[0][i];
      if (out_off.h_dim_off1 != NULL) out_off.h_dim_off1[thread_off+i] = temp[1][i];
      if (out_off.h_dim_off2 != NULL) out_off.h_dim_off2[thread_off+i] = temp[2][i];
      if (out_off.h_dim_off3 != NULL) out_off.h_dim_off3[thread_off+i] = temp[3][i];
      if (out_off.h_dim_off4 != NULL) out_off.h_dim_off4[thread_off+i] = temp[4][i];
    }

  }, simple_partitioner());
}

void probe_CPU2(struct offsetCPU in_off, struct probeArgsCPU pargs, struct offsetCPU out_off, int num_tuples,
  int* total, int start_offset = 0) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(in_off.h_lo_off != NULL);
  assert(out_off.h_lo_off != NULL);

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) { //make sure the grainsize is not too big (will result in segfault on temp[5][end-start] -> not enough stack memory)
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    //printf("worker_index = %d %d\n", worker_index, end-start);
    unsigned int temp[5][end-start];
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    unsigned int count = 0;
    
    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int slot1 = 1, slot2 = 1, slot3 = 1, slot4 = 1;
        int lo_offset;

        lo_offset = in_off.h_lo_off[start_offset + i];

        if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          slot1 = slot >> 32;
        } else if (in_off.h_dim_off1 != NULL) slot1 = in_off.h_dim_off1[start_offset + i] + 1;


        if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          slot2 = slot >> 32;
        } else if (in_off.h_dim_off2 != NULL) slot2 = in_off.h_dim_off2[start_offset + i] + 1;


        if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          slot3 = slot >> 32;
        } else if (in_off.h_dim_off3 != NULL) slot3 = in_off.h_dim_off3[start_offset + i] + 1;


        if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          slot4 = slot >> 32;
        } else if (in_off.h_dim_off4 != NULL) slot4 = in_off.h_dim_off4[start_offset + i] + 1;


        temp[0][count] = lo_offset;
        temp[1][count] = slot1-1;
        temp[2][count] = slot2-1;
        temp[3][count] = slot3-1;
        temp[4][count] = slot4-1;
        count++;


      }
    }

    for (int i = end_batch ; i < end; i++) {
        int hash;
        long long slot;
        int slot1 = 1, slot2 = 1, slot3 = 1, slot4 = 1;
        int lo_offset;

        lo_offset = in_off.h_lo_off[start_offset + i];

        if (pargs.ht1 != NULL && pargs.key_col1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          slot1 = slot >> 32;
        } else if (in_off.h_dim_off1 != NULL) slot1 = in_off.h_dim_off1[start_offset + i] + 1;


        if (pargs.ht2 != NULL && pargs.key_col2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          slot2 = slot >> 32;
        } else if (in_off.h_dim_off2 != NULL) slot2 = in_off.h_dim_off2[start_offset + i] + 1;


        if (pargs.ht3 != NULL && pargs.key_col3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          slot3 = slot >> 32;
        } else if (in_off.h_dim_off3 != NULL) slot3 = in_off.h_dim_off3[start_offset + i] + 1;


        if (pargs.ht4 != NULL && pargs.key_col4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          slot4 = slot >> 32;
        } else if (in_off.h_dim_off4 != NULL) slot4 = in_off.h_dim_off4[start_offset + i] + 1;


        temp[0][count] = lo_offset;
        temp[1][count] = slot1-1;
        temp[2][count] = slot2-1;
        temp[3][count] = slot3-1;
        temp[4][count] = slot4-1;
        count++;
    }

    int thread_off = __atomic_fetch_add(total, count, __ATOMIC_RELAXED);

    for (int i = 0; i < count; i++) {
      assert(out_off.h_lo_off != NULL);
      if (out_off.h_lo_off != NULL) out_off.h_lo_off[thread_off+i] = temp[0][i];
      if (out_off.h_dim_off1 != NULL) out_off.h_dim_off1[thread_off+i] = temp[1][i];
      if (out_off.h_dim_off2 != NULL) out_off.h_dim_off2[thread_off+i] = temp[2][i];
      if (out_off.h_dim_off3 != NULL) out_off.h_dim_off3[thread_off+i] = temp[3][i];
      if (out_off.h_dim_off4 != NULL) out_off.h_dim_off4[thread_off+i] = temp[4][i];
    }

  }, simple_partitioner());
}


void probe_group_by_CPU(
  struct probeArgsCPU pargs,  struct groupbyArgsCPU gargs, int num_tuples, 
  int* res, int start_offset = 0, short* segment_group = NULL) {


  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);


  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          dim_val1 = slot;
        }

        if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          dim_val2 = slot;
        }

        if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          dim_val3 = slot;
        }

        if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          dim_val4 = slot;
        }

        hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
        if (dim_val1 != 0) res[hash * 6] = dim_val1;
        if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
        if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
        if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

        int aggr1 = 0; int aggr2 = 0;
        if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
        int temp = (*(gargs.h_group_func))(aggr1, aggr2);
        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   temp = gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset];
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset];
        // } else assert(0);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        if (slot == 0) continue;
        dim_val1 = slot;
      }

      if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        if (slot == 0) continue;
        dim_val2 = slot;
      }

      if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        if (slot == 0) continue;
        dim_val3 = slot;
      }

      if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
        dim_val4 = slot;
      }

      hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
      if (dim_val1 != 0) res[hash * 6] = dim_val1;
      if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
      if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
      if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

      int aggr1 = 0; int aggr2 = 0;
      if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
      if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
      int temp = (*(gargs.h_group_func))(aggr1, aggr2);
      // if (gargs.mode == 0) {
      //   assert(gargs.aggr_col1 != NULL);
      //   temp = gargs.aggr_col1[lo_offset];
      // } else if (gargs.mode == 1) {
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   temp = gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset];
      // } else if  (gargs.mode == 2){ 
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   temp = gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset];
      // } else assert(0);

      __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
    }
  }, simple_partitioner());

}

void probe_group_by_CPU2(struct offsetCPU offset,
  struct probeArgsCPU pargs,  struct groupbyArgsCPU gargs, int num_tuples,
  int* res, int start_offset = 0) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(offset.h_lo_off != NULL);

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
        int lo_offset;

        lo_offset = offset.h_lo_off[start_offset + i];

        if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          dim_val1 = slot;
        } else if (gargs.group_col1 != NULL) {
          assert(offset.h_dim_off1 != NULL);
          dim_val1 = gargs.group_col1[offset.h_dim_off1[start_offset + i]];
        }

        if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          dim_val2 = slot;
        } else if (gargs.group_col2 != NULL) {
          assert(offset.h_dim_off2 != NULL);
          dim_val2 = gargs.group_col2[offset.h_dim_off2[start_offset + i]];
        }

        if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          dim_val3 = slot;
        } else if (gargs.group_col3 != NULL) {
          assert(offset.h_dim_off3 != NULL);
          dim_val3 = gargs.group_col3[offset.h_dim_off3[start_offset + i]];
        }

        if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          dim_val4 = slot;
        } else if (gargs.group_col4 != NULL) {
          assert(offset.h_dim_off4 != NULL);
          dim_val4 = gargs.group_col4[offset.h_dim_off4[start_offset + i]];
        }

        hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
        if (dim_val1 != 0) res[hash * 6] = dim_val1;
        if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
        if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
        if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

        int aggr1 = 0; int aggr2 = 0;
        if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
        int temp = (*(gargs.h_group_func))(aggr1, aggr2);
        // int temp;
        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   temp = gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset];
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset];
        // } else assert(0);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
      }
    }

    for (int i = end_batch ; i < end; i++) {

        int hash;
        long long slot;
        int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
        int lo_offset;

        lo_offset = offset.h_lo_off[start_offset + i];

        if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          dim_val1 = slot;
        } else if (gargs.group_col1 != NULL) {
          assert(offset.h_dim_off1 != NULL);
          dim_val1 = gargs.group_col1[offset.h_dim_off1[start_offset + i]];
        }

        if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          dim_val2 = slot;
        } else if (gargs.group_col2 != NULL) {
          assert(offset.h_dim_off2 != NULL);
          dim_val2 = gargs.group_col2[offset.h_dim_off2[start_offset + i]];
        }

        if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          dim_val3 = slot;
        } else if (gargs.group_col3 != NULL) {
          assert(offset.h_dim_off3 != NULL);
          dim_val3 = gargs.group_col3[offset.h_dim_off3[start_offset + i]];
        }

        if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          dim_val4 = slot;
        } else if (gargs.group_col4 != NULL) {
          assert(offset.h_dim_off4 != NULL);
          dim_val4 = gargs.group_col4[offset.h_dim_off4[start_offset + i]];
        }

        hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
        if (dim_val1 != 0) res[hash * 6] = dim_val1;
        if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
        if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
        if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

        int aggr1 = 0; int aggr2 = 0;
        if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
        int temp = (*(gargs.h_group_func))(aggr1, aggr2);
        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   temp = gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset];
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset];
        // } else assert(0);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
    }
  }, simple_partitioner());

}

void filter_probe_group_by_CPU(
  struct filterArgsCPU fargs, struct probeArgsCPU pargs, struct groupbyArgsCPU gargs,
  int num_tuples, int* res, int start_offset = 0, short* segment_group = NULL, struct statsCPU stats) {


  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);

  int task_count = num_tuples/TASK_SIZE;

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    int count_filter1[2] = {0}, count_filter2[2] = {0};
    int count_probe1[2] = {0}, count_probe2[2] = {0}, count_probe3[2] = {0}, count_probe4[2] = {0};

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (fargs.filter_col1 != NULL) {
          if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
        }

        if (fargs.filter_col2 != NULL) {
          if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
        }

        if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          dim_val1 = slot;
        }

        if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          dim_val2 = slot;
        }

        if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          dim_val3 = slot;
        }

        if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          dim_val4 = slot;
        }

        hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
        if (dim_val1 != 0) res[hash * 6] = dim_val1;
        if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
        if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
        if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

        int aggr1 = 0; int aggr2 = 0;
        if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
        int temp = (*(gargs.h_group_func))(aggr1, aggr2);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      if (fargs.filter_col1 != NULL) {
        if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
      }

      if (fargs.filter_col2 != NULL) {
        if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
      }

      if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        if (slot == 0) continue;
        dim_val1 = slot;
      }

      if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        if (slot == 0) continue;
        dim_val2 = slot;
      }

      if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        if (slot == 0) continue;
        dim_val3 = slot;
      }

      if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
        dim_val4 = slot;
      }

      hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
      if (dim_val1 != 0) res[hash * 6] = dim_val1;
      if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
      if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
      if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

      int aggr1 = 0; int aggr2 = 0;
      if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
      if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
      int temp = (*(gargs.h_group_func))(aggr1, aggr2);

      __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
    }

    for  (int i = start_segment; i < end_segment; i++) {
      __atomic_fetch_add(&(stats.count[fargs.filter_id1][i]), count_filter1[i-start_segment], __ATOMIC_RELAXED);
      __atomic_fetch_add(&(stats.count[fargs.filter_id2][i]), count_filter2[i-start_segment], __ATOMIC_RELAXED);
      __atomic_fetch_add(&(stats.count[pargs.probe_id1][i]), count_probe1[i-start_segment], __ATOMIC_RELAXED);
      __atomic_fetch_add(&(stats.count[pargs.probe_id2][i]), count_probe2[i-start_segment], __ATOMIC_RELAXED);
      __atomic_fetch_add(&(stats.count[pargs.probe_id3][i]), count_probe3[i-start_segment], __ATOMIC_RELAXED);
      __atomic_fetch_add(&(stats.count[pargs.probe_id4][i]), count_probe4[i-start_segment], __ATOMIC_RELAXED);
    }

  }, simple_partitioner());

}

void filter_probe_group_by_CPU4(struct offsetCPU offset,
  struct filterArgsCPU fargs, struct probeArgsCPU pargs, struct groupbyArgsCPU gargs,
  int num_tuples, int* res, int start_offset = 0, struct statsCPU stats) {

  assert(offset.h_lo_off != NULL);

  int task_count = (num_tuples + TASK_SIZE - 1)/TASK_SIZE;
  int rem_task = (num_tuples % TASK_SIZE == 0) ? (TASK_SIZE):(num_tuples % TASK_SIZE)

  // Probe
  parallel_for(blocked_range<size_t>(0, task_count), [&](auto range) {
    unsigned int start_task = range.begin();
    unsigned int end_task = range.end();

    int count_filter1[TOT_SEGMENT] = {0}, count_filter2[TOT_SEGMENT] = {0};
    int count_probe1[TOT_SEGMENT] = {0}, count_probe2[TOT_SEGMENT] = {0};
    int count_probe3[TOT_SEGMENT] = {0}, count_probe4[TOT_SEGMENT] = {0};
    int count_group1[TOT_SEGMENT] = {0}, count_group2[TOT_SEGMENT] = {0};
    int count_group3[TOT_SEGMENT] = {0}, count_group4[TOT_SEGMENT] = {0};
    int count_aggr1[TOT_SEGMENT] = {0}, count_aggr2[TOT_SEGMENT] = {0};
    bool count_check[TOT_SEGMENT] = {0};

    for (task = start_task; task < end_task; task++) {
          unsigned int start = task * TASK_SIZE;
          unsigned int end = (end_task == task_count - 1) ? (task * TASK_SIZE + rem_task):(task * TASK_SIZE + TASK_SIZE);
          unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

          for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
            #pragma simd
            for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
              int hash;
              long long slot;
              int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
              int lo_offset;

              lo_offset = offset.h_lo_off[start_offset + i];
              count_check[lo_offset / SEGMENT_SIZE] = 1;

              if (fargs.filter_col1 != NULL) {
                count_filter1[lo_offset / SEGMENT_SIZE]++;
                if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
              }

              if (fargs.filter_col2 != NULL) {
                count_filter2[lo_offset / SEGMENT_SIZE]++;
                if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
              }

              if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
                count_probe1[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
                slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
                if (slot == 0) continue;
                dim_val1 = slot;
              } else if (gargs.group_col1 != NULL) {
                assert(offset.h_dim_off1 != NULL);
                count_group1[offset.h_dim_off1[start_offset + i] / SEGMENT_SIZE]++;
                dim_val1 = gargs.group_col1[offset.h_dim_off1[start_offset + i]];
              }

              if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
                count_probe2[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
                slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
                if (slot == 0) continue;
                dim_val2 = slot;
              } else if (gargs.group_col2 != NULL) {
                assert(offset.h_dim_off2 != NULL);
                count_group2[offset.h_dim_off2[start_offset + i] / SEGMENT_SIZE]++;
                dim_val2 = gargs.group_col2[offset.h_dim_off2[start_offset + i]];
              }

              if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
                count_probe3[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
                slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
                if (slot == 0) continue;
                dim_val3 = slot;
              } else if (gargs.group_col3 != NULL) {
                assert(offset.h_dim_off3 != NULL);
                count_group3[offset.h_dim_off3[start_offset + i] / SEGMENT_SIZE]++;
                dim_val3 = gargs.group_col3[offset.h_dim_off3[start_offset + i]];
              }

              if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
                count_probe4[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
                slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
                if (slot == 0) continue;
                dim_val4 = slot;
              } else if (gargs.group_col4 != NULL) {
                assert(offset.h_dim_off4 != NULL);
                count_group4[offset.h_dim_off4[start_offset + i] / SEGMENT_SIZE]++;
                dim_val4 = gargs.group_col4[offset.h_dim_off4[start_offset + i]];
              }

              hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
              if (dim_val1 != 0) res[hash * 6] = dim_val1;
              if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
              if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
              if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

              int aggr1 = 0; int aggr2 = 0;
              if (gargs.aggr_col1 != NULL) {
                count_aggr1[lo_offset / SEGMENT_SIZE]++;
                aggr1 = gargs.aggr_col1[lo_offset];
              }
              if (gargs.aggr_col2 != NULL) {
                count_aggr2[lo_offset / SEGMENT_SIZE]++;
                aggr2 = gargs.aggr_col2[lo_offset];
              }
              int temp = (*(gargs.h_group_func))(aggr1, aggr2);

              __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
            }
          }

          for (int i = end_batch ; i < end; i++) {

              int hash;
              long long slot;
              int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
              int lo_offset;

              lo_offset = offset.h_lo_off[start_offset + i];
              count_check[lo_offset / SEGMENT_SIZE] = 1;

              if (fargs.filter_col1 != NULL) {
                count_filter1[lo_offset / SEGMENT_SIZE]++;
                if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
              }

              if (fargs.filter_col2 != NULL) {
                count_filter2[lo_offset / SEGMENT_SIZE]++;
                if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
              }

              if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
                count_probe1[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
                slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
                if (slot == 0) continue;
                dim_val1 = slot;
              } else if (gargs.group_col1 != NULL) {
                assert(offset.h_dim_off1 != NULL);
                count_group1[offset.h_dim_off1[start_offset + i] / SEGMENT_SIZE]++;
                dim_val1 = gargs.group_col1[offset.h_dim_off1[start_offset + i]];
              }

              if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
                count_probe2[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
                slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
                if (slot == 0) continue;
                dim_val2 = slot;
              } else if (gargs.group_col2 != NULL) {
                assert(offset.h_dim_off2 != NULL);
                count_group2[offset.h_dim_off2[start_offset + i] / SEGMENT_SIZE]++;
                dim_val2 = gargs.group_col2[offset.h_dim_off2[start_offset + i]];
              }

              if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
                count_probe3[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
                slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
                if (slot == 0) continue;
                dim_val3 = slot;
              } else if (gargs.group_col3 != NULL) {
                assert(offset.h_dim_off3 != NULL);
                count_group3[offset.h_dim_off3[start_offset + i] / SEGMENT_SIZE]++;
                dim_val3 = gargs.group_col3[offset.h_dim_off3[start_offset + i]];
              }

              if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
                count_probe4[lo_offset / SEGMENT_SIZE]++;
                hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
                slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
                if (slot == 0) continue;
                dim_val4 = slot;
              } else if (gargs.group_col4 != NULL) {
                assert(offset.h_dim_off4 != NULL);
                count_group4[offset.h_dim_off4[start_offset + i] / SEGMENT_SIZE]++;
                dim_val4 = gargs.group_col4[offset.h_dim_off4[start_offset + i]];
              }

              hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
              if (dim_val1 != 0) res[hash * 6] = dim_val1;
              if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
              if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
              if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

              int aggr1 = 0; int aggr2 = 0;
              if (gargs.aggr_col1 != NULL) {
                count_aggr1[lo_offset / SEGMENT_SIZE]++;
                aggr1 = gargs.aggr_col1[lo_offset];
              }
              if (gargs.aggr_col2 != NULL) {
                count_aggr2[lo_offset / SEGMENT_SIZE]++;
                aggr2 = gargs.aggr_col2[lo_offset];
              }
              int temp = (*(gargs.h_group_func))(aggr1, aggr2);

              __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
          }

          for  (int i = 0; i < TOT_SEGMENT; i++) {
            if (count_check[i]) {
              __atomic_fetch_add(&(stats.h_count[fargs.filter_id1][i]), count_filter1[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[fargs.filter_id2][i]), count_filter2[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.probe_id1][i]), count_probe1[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.probe_id2][i]), count_probe2[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.probe_id3][i]), count_probe3[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.probe_id4][i]), count_probe4[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.group_id1][i]), count_group1[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.group_id2][i]), count_group2[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.group_id3][i]), count_group3[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.group_id4][i]), count_group4[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.aggr_id1][i]), count_aggr1[i], __ATOMIC_RELAXED);
              __atomic_fetch_add(&(stats.h_count[pargs.aggr_id2][i]), count_aggr2[i], __ATOMIC_RELAXED);
            }
          }


    }

  }, simple_partitioner());

}

void build_CPU(struct filterArgsCPU fargs,
  struct buildArgsCPU bargs, int num_tuples, int* hash_table,
  int start_offset = 0, short* segment_group = NULL) {

  assert(bargs.key_col != NULL);
  assert(hash_table != NULL);
  assert(segment_group != NULL);

  int grainsize = num_tuples/NUM_THREADS + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {

    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    int start_segment, start_group, segment_idx;
    int end_segment;

    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    // cout << start_segment << " " << end_segment << endl;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int table_offset;
        int flag = 1;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        table_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (fargs.filter_col1 != NULL) {
          // if (fargs.mode1 == 0)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1);
          // else if (fargs.mode1 == 1)
          //   flag = (fargs.filter_col1[table_offset] >= fargs.compare1 && fargs.filter_col1[table_offset] <= fargs.compare2);
          // else if (fargs.mode1 == 2)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1 || fargs.filter_col1[table_offset] == fargs.compare2);

          flag = (*(fargs.h_filter_func1))(fargs.filter_col1[table_offset], fargs.compare1, fargs.compare2);

        }

        if (flag) {
          int key = bargs.key_col[table_offset];
          int hash = HASH(key, bargs.num_slots, bargs.val_min);
          hash_table[(hash << 1) + 1] = table_offset + 1;
          if (bargs.val_col != NULL) hash_table[hash << 1] = bargs.val_col[table_offset];
        }

      }
    }

    for (int i = end_batch ; i < end; i++) {
      int table_offset;
      int flag = 1;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      table_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (fargs.filter_col1 != NULL) {
          // if (fargs.mode1 == 0)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1);
          // else if (fargs.mode1 == 1)
          //   flag = (fargs.filter_col1[table_offset] >= fargs.compare1 && fargs.filter_col1[table_offset] <= fargs.compare2);
          // else if (fargs.mode1 == 2)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1 || fargs.filter_col1[table_offset] == fargs.compare2);

          flag = (*(fargs.h_filter_func1))(fargs.filter_col1[table_offset], fargs.compare1, fargs.compare2);
        }

      if (flag) {
        int key = bargs.key_col[table_offset];
        int hash = HASH(key, bargs.num_slots, bargs.val_min);
        hash_table[(hash << 1) + 1] = table_offset + 1;
        if (bargs.val_col != NULL) hash_table[hash << 1] = bargs.val_col[table_offset];
      }
    }

  });
}

void build_CPU2(int *dim_off, struct filterArgsCPU fargs,
  struct buildArgsCPU bargs, int num_tuples, int* hash_table,
  int start_offset = 0) {

  assert(bargs.key_col != NULL);
  assert(hash_table != NULL);
  assert(dim_off != NULL);

  int grainsize = num_tuples/NUM_THREADS + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {

    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int table_offset;
        // int flag = 1;

        table_offset = dim_off[start_offset + i];

        // if (fargs.filter_col1 != NULL) {
          // if (fargs.mode1 == 0)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1);
          // else if (fargs.mode1 == 1)
          //   flag = (fargs.filter_col1[table_offset] >= fargs.compare1 && fargs.filter_col1[table_offset] <= fargs.compare2);
          // else if (fargs.mode1 == 2)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1 || fargs.filter_col1[table_offset] == fargs.compare2);

          // flag = (*(fargs.h_filter_func1))(fargs.filter_col1[table_offset], fargs.compare1, fargs.compare2);
        // }

        // if (flag) {
          int key = bargs.key_col[table_offset];
          int hash = HASH(key, bargs.num_slots, bargs.val_min);
          hash_table[(hash << 1) + 1] = table_offset + 1;
          if (bargs.val_col != NULL) hash_table[hash << 1] = bargs.val_col[table_offset];
        // }

      }
    }

    for (int i = end_batch ; i < end; i++) {
      int table_offset;
      // int flag = 1;

      table_offset = dim_off[start_offset + i];

        // if (fargs.filter_col1 != NULL) {
          // if (fargs.mode1 == 0)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1);
          // else if (fargs.mode1 == 1)
          //   flag = (fargs.filter_col1[table_offset] >= fargs.compare1 && fargs.filter_col1[table_offset] <= fargs.compare2);
          // else if (fargs.mode1 == 2)
          //   flag = (fargs.filter_col1[table_offset] == fargs.compare1 || fargs.filter_col1[table_offset] == fargs.compare2);

          // flag = (*(fargs.h_filter_func1))(fargs.filter_col1[table_offset], fargs.compare1, fargs.compare2);
        // }

      // if (flag) {
        int key = bargs.key_col[table_offset];
        int hash = HASH(key, bargs.num_slots, bargs.val_min);
        hash_table[(hash << 1) + 1] = table_offset + 1;
        if (bargs.val_col != NULL) hash_table[hash << 1] = bargs.val_col[table_offset];
      // }
    }

  }, simple_partitioner());
}


void filter_CPU(struct filterArgsCPU fargs,
  int* out_off, int num_tuples, int* total,
  int start_offset = 0, short* segment_group = NULL) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    int count = 0;
    int temp[end-start];

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        bool selection_flag = 1;
        int col_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;   

        col_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (fargs.filter_col1 != NULL) {
          // if (fargs.mode1 == 0)
          //   selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1);
          // else if (fargs.mode1 == 1)
          //   selection_flag = (fargs.filter_col1[col_offset] >= fargs.compare1 && fargs.filter_col1[col_offset] <= fargs.compare2);
          // else if (fargs.mode1 == 2)
          //   selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1 || fargs.filter_col1[col_offset] == fargs.compare2);

          selection_flag = (*(fargs.h_filter_func1))(fargs.filter_col1[col_offset], fargs.compare1, fargs.compare2);
        }

        if (fargs.filter_col2 != NULL) {
          // if (fargs.mode2 == 0)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3);
          // else if (fargs.mode2 == 1)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] >= fargs.compare3 && fargs.filter_col2[col_offset] <= fargs.compare4);
          // else if (fargs.mode2 == 2)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3 || fargs.filter_col2[col_offset] == fargs.compare4);

          selection_flag = selection_flag && (*(fargs.h_filter_func2))(fargs.filter_col2[col_offset], fargs.compare3, fargs.compare4);
        }

        if (selection_flag) {
          temp[count] = col_offset;
          count++;        
        }
      }
    }

    for (int i = end_batch ; i < end; i++) {
      bool selection_flag = 1;
      int col_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;   

      col_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (fargs.filter_col1 != NULL) {
          // if (fargs.mode1 == 0)
          //   selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1);
          // else if (fargs.mode1 == 1)
          //   selection_flag = (fargs.filter_col1[col_offset] >= fargs.compare1 && fargs.filter_col1[col_offset] <= fargs.compare2);
          // else if (fargs.mode1 == 2)
          //   selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1 || fargs.filter_col1[col_offset] == fargs.compare2);

          selection_flag = (*(fargs.h_filter_func1))(fargs.filter_col1[col_offset], fargs.compare1, fargs.compare2);
        }

        if (fargs.filter_col2 != NULL) {
          // if (fargs.mode2 == 0)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3);
          // else if (fargs.mode2 == 1)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] >= fargs.compare3 && fargs.filter_col2[col_offset] <= fargs.compare4);
          // else if (fargs.mode2 == 2)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3 || fargs.filter_col2[col_offset] == fargs.compare4);

          selection_flag = selection_flag && (*(fargs.h_filter_func2))(fargs.filter_col2[col_offset], fargs.compare3, fargs.compare4);
        }

      if (selection_flag) {
        temp[count] = col_offset;
        count++;
      }
    }

    int thread_off = __atomic_fetch_add(total, count, __ATOMIC_RELAXED);

    assert(out_off != NULL);
    for (int i = 0; i < count; i++) {
      out_off[thread_off+i] = temp[i];
    }

  }, simple_partitioner());
}

void filter_CPU2(int* off_col, struct filterArgsCPU fargs,
  int* out_off, int num_tuples, int* total,
  int start_offset = 0) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(off_col != NULL);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    int count = 0;
    int temp[end-start];

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        bool selection_flag = 1;
        int col_offset;

        col_offset = off_col[start_offset + i];

        // if (fargs.filter_col1 != NULL) {
        //   if (fargs.mode1 == 0)
        //     selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1);
        //   else if (fargs.mode1 == 1)
        //     selection_flag = (fargs.filter_col1[col_offset] >= fargs.compare1 && fargs.filter_col1[col_offset] <= fargs.compare2);
        //   else if (fargs.mode1 == 2)
        //     selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1 || fargs.filter_col1[col_offset] == fargs.compare2);

            // selection_flag = (*(fargs.h_filter_func1))(fargs.filter_col1[col_offset], fargs.compare1, fargs.compare2);
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (fargs.mode2 == 0)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3);
          // else if (fargs.mode2 == 1)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] >= fargs.compare3 && fargs.filter_col2[col_offset] <= fargs.compare4);
          // else if (fargs.mode2 == 2)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3 || fargs.filter_col2[col_offset] == fargs.compare4);

          selection_flag = selection_flag && (*(fargs.h_filter_func2))(fargs.filter_col2[col_offset], fargs.compare3, fargs.compare4);
        // }

        if (selection_flag) {
          temp[count] = col_offset;
          count++;        
        }
      }
    }

    for (int i = end_batch ; i < end; i++) {
      bool selection_flag = 1;
      int col_offset;

      col_offset = off_col[start_offset + i];

        // if (fargs.filter_col1 != NULL) {
        //   if (fargs.mode1 == 0)
        //     selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1);
        //   else if (fargs.mode1 == 1)
        //     selection_flag = (fargs.filter_col1[col_offset] >= fargs.compare1 && fargs.filter_col1[col_offset] <= fargs.compare2);
        //   else if (fargs.mode1 == 2)
        //     selection_flag = (fargs.filter_col1[col_offset] == fargs.compare1 || fargs.filter_col1[col_offset] == fargs.compare2);

            // selection_flag = (*(fargs.h_filter_func1))(fargs.filter_col1[col_offset], fargs.compare1, fargs.compare2);
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (fargs.mode2 == 0)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3);
          // else if (fargs.mode2 == 1)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] >= fargs.compare3 && fargs.filter_col2[col_offset] <= fargs.compare4);
          // else if (fargs.mode2 == 2)
          //   selection_flag = selection_flag && (fargs.filter_col2[col_offset] == fargs.compare3 || fargs.filter_col2[col_offset] == fargs.compare4);

          selection_flag = selection_flag && (*(fargs.h_filter_func2))(fargs.filter_col2[col_offset], fargs.compare3, fargs.compare4);
        // }

      if (selection_flag) {
        temp[count] = col_offset;
        count++;
      }
    }

    int thread_off = __atomic_fetch_add(total, count, __ATOMIC_RELAXED);

    assert(out_off != NULL);
    for (int i = 0; i < count; i++) {
      out_off[thread_off+i] = temp[i];
    }

  }, simple_partitioner());
}

void groupByCPU(struct offsetCPU offset, 
  struct groupbyArgsCPU gargs, int num_tuples, int* res) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    int start = range.begin();
    int end = range.end();
    int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int groupval1 = 0, groupval2 = 0, groupval3 = 0, groupval4 = 0;
        int aggrval1 = 0, aggrval2 = 0;

        if (gargs.group_col1 != NULL) {
          assert(offset.h_dim_off1 != NULL);
          groupval1 = gargs.group_col1[offset.h_dim_off1[i]];
        }

        if (gargs.group_col2 != NULL) {
          assert(offset.h_dim_off2 != NULL);
          groupval2 = gargs.group_col2[offset.h_dim_off2[i]];
        }

        if (gargs.group_col3 != NULL) {
          assert(offset.h_dim_off3 != NULL);
          groupval3 = gargs.group_col3[offset.h_dim_off3[i]];
        }

        if (gargs.group_col4 != NULL) {
          assert(offset.h_dim_off4 != NULL);
          groupval4 = gargs.group_col4[offset.h_dim_off4[i]];
        }

        assert(offset.h_lo_off != NULL);
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[offset.h_lo_off[i]];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[offset.h_lo_off[i]];

        int hash = ((groupval1 - gargs.min_val1) * gargs.unique_val1 + (groupval2 - gargs.min_val2) * gargs.unique_val2 +  (groupval3 - gargs.min_val3) * gargs.unique_val3 + (groupval4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;

        if (groupval1 != 0) res[hash * 6] = groupval1;
        if (groupval2 != 0) res[hash * 6 + 1] = groupval2;
        if (groupval3 != 0) res[hash * 6 + 2] = groupval3;
        if (groupval4 != 0) res[hash * 6 + 3] = groupval4;

        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[offset.h_lo_off[i]];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[offset.h_lo_off[i]];
        int temp = (*(gargs.h_group_func))(aggrval1, aggrval2);
        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   temp = aggrval1;
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = aggrval1 - aggrval2;
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = aggrval1 * aggrval2;
        // } else assert(0);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
      }
    }
    for (int i = end_batch ; i < end; i++) {
        int groupval1 = 0, groupval2 = 0, groupval3 = 0, groupval4 = 0;
        int aggrval1 = 0, aggrval2 = 0;

        if (gargs.group_col1 != NULL) {
          assert(offset.h_dim_off1 != NULL);
          groupval1 = gargs.group_col1[offset.h_dim_off1[i]];
        }

        if (gargs.group_col2 != NULL) {
          assert(offset.h_dim_off2 != NULL);
          groupval2 = gargs.group_col2[offset.h_dim_off2[i]];
        }

        if (gargs.group_col3 != NULL) {
          assert(offset.h_dim_off3 != NULL);
          groupval3 = gargs.group_col3[offset.h_dim_off3[i]];
        }

        if (gargs.group_col4 != NULL) {
          assert(offset.h_dim_off4 != NULL);
          groupval4 = gargs.group_col4[offset.h_dim_off4[i]];
        }

        assert(offset.h_lo_off != NULL);
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[offset.h_lo_off[i]];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[offset.h_lo_off[i]];

        int hash = ((groupval1 - gargs.min_val1) * gargs.unique_val1 + (groupval2 - gargs.min_val2) * gargs.unique_val2 +  (groupval3 - gargs.min_val3) * gargs.unique_val3 + (groupval4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;

        if (groupval1 != 0) res[hash * 6] = groupval1;
        if (groupval2 != 0) res[hash * 6 + 1] = groupval2;
        if (groupval3 != 0) res[hash * 6 + 2] = groupval3;
        if (groupval4 != 0) res[hash * 6 + 3] = groupval4;

        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[offset.h_lo_off[i]];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[offset.h_lo_off[i]];
        int temp = (*(gargs.h_group_func))(aggrval1, aggrval2);
        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   temp = aggrval1;
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = aggrval1 - aggrval2;
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   temp = aggrval1 * aggrval2;
        // } else assert(0);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
    }
  });
}

void aggregationCPU(int* lo_off, 
  struct groupbyArgsCPU gargs, int num_tuples, int* res) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    int start = range.begin();
    int end = range.end();
    int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    long long local_sum = 0;

    assert(lo_off != NULL);

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int aggrval1 = 0, aggrval2 = 0;

        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_off[i]];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_off[i]];

        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   local_sum += aggrval1;
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   local_sum += aggrval1 - aggrval2;
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
          // local_sum += aggrval1 * aggrval2;
        // } else assert(0);

        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);

      }
    }
    for (int i = end_batch ; i < end; i++) {
      int aggrval1 = 0, aggrval2 = 0;

      assert(lo_off != NULL);
      if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_off[i]];
      if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_off[i]];

      // if (gargs.mode == 0) {
      //   assert(gargs.aggr_col1 != NULL);
      //   local_sum += aggrval1;
      // } else if (gargs.mode == 1) {
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   local_sum += (aggrval1 - aggrval2);
      // } else if  (gargs.mode == 2){ 
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        // local_sum += (aggrval1 * aggrval2);
      // } else assert(0);

      local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);

    }

    __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[4]), (long long)(local_sum), __ATOMIC_RELAXED);

  });
}


void probe_aggr_CPU(
  struct probeArgsCPU pargs, struct groupbyArgsCPU gargs, int num_tuples,
  int* res, int start_offset = 0, short* segment_group = NULL) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    long long local_sum = 0;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
        // }

        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   local_sum += gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
        // } else if (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
          // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
        // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);
      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
      //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
      //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
      //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
      //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
      //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
      //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
      // }

      // if (gargs.mode == 0) {
      //   assert(gargs.aggr_col1 != NULL);
      //   local_sum += gargs.aggr_col1[lo_offset];
      // } else if (gargs.mode == 1) {
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
      // } else if (gargs.mode == 2){ 
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
      // } else assert(0);

      int aggrval1 = 0, aggrval2 = 0;
      if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
      if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
      local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);
    }

    __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[4]), (long long)(local_sum), __ATOMIC_RELAXED);

  }, simple_partitioner());

}

void probe_aggr_CPU2(struct offsetCPU offset,
  struct probeArgsCPU pargs, struct groupbyArgsCPU gargs, 
  int num_tuples, int* res, int start_offset = 0) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(offset.h_lo_off != NULL);

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    long long local_sum = 0;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int lo_offset;

        lo_offset = offset.h_lo_off[start_offset + i];

        // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
        // }

        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   local_sum += gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
        // } else if  (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
          // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
        // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);

      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int lo_offset;

      lo_offset = offset.h_lo_off[start_offset + i];

      // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
      //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
      //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
      //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
      //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
      //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
      //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
      // }

      // if (gargs.mode == 0) {
      //   assert(gargs.aggr_col1 != NULL);
      //   local_sum += gargs.aggr_col1[lo_offset];
      // } else if (gargs.mode == 1) {
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
      // } else if (gargs.mode == 2){ 
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
      // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);
    }

    __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[4]), (long long)(local_sum), __ATOMIC_RELAXED);

  }, simple_partitioner());

}

void filter_probe_aggr_CPU(
  struct filterArgsCPU fargs, struct probeArgsCPU pargs, struct groupbyArgsCPU gargs,
  int num_tuples, int* res, int start_offset = 0, short* segment_group = NULL) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    long long local_sum = 0;

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        // lo_offset = start_offset + i;

        // bool selection_flag;
        // selection_flag = (pargs.key_col4[i] > 19930000 && pargs.key_col4[i] < 19940000);
        // selection_flag = selection_flag && (fargs.filter_col1[i] >= fargs.compare1 && fargs.filter_col1[i] <= fargs.compare2);
        // selection_flag = selection_flag && (fargs.filter_col2[i] >= fargs.compare3 && fargs.filter_col2[i] <= fargs.compare4);

        // if (selection_flag) local_sum += (gargs.aggr_col1[i] * gargs.aggr_col2[i]);

        // if (!(pargs.key_col4[lo_offset] > 19930000 && pargs.key_col4[lo_offset] < 19940000)) continue;

        // if (fargs.filter_col1 != NULL) {
          // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
        // }

        // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
        // }

        // if (!(pargs.key_col4[lo_offset] > 19930000 && pargs.key_col4[lo_offset] < 19940000)) continue;

        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   local_sum += gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
        // } else if  (gargs.mode == 2){ 
          // assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
          // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
        // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);

      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      // lo_offset = start_offset + i;

      // bool selection_flag;
      // selection_flag = (pargs.key_col4[i] > 19930000 && pargs.key_col4[i] < 19940000);
      // selection_flag = selection_flag && (fargs.filter_col1[i] >= fargs.compare1 && fargs.filter_col1[i] <= fargs.compare2);
      // selection_flag = selection_flag && (fargs.filter_col2[i] >= fargs.compare3 && fargs.filter_col2[i] <= fargs.compare4);

      // if (selection_flag) local_sum += (gargs.aggr_col1[i] * gargs.aggr_col2[i]);

      // if (!(pargs.key_col4[lo_offset] > 19930000 && pargs.key_col4[lo_offset] < 19940000)) continue;

      // if (fargs.filter_col1 != NULL) {
        // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
        if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
      // }

      // if (fargs.filter_col2 != NULL) {
        // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
        if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
      // }

      // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
      //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
      //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
      //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
      //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
      //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
      //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
      // }

      // if (!(pargs.key_col4[lo_offset] > 19930000 && pargs.key_col4[lo_offset] < 19940000)) continue;

      // if (gargs.mode == 0) {
      //   assert(gargs.aggr_col1 != NULL);
      //   local_sum += gargs.aggr_col1[lo_offset];
      // } else if (gargs.mode == 1) {
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
      // } else if  (gargs.mode == 2){ 
        // assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
      // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);

    }

    __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[4]), (long long)(local_sum), __ATOMIC_RELAXED);

  }, simple_partitioner());

}

void filter_probe_aggr_CPU2(struct offsetCPU offset,
  struct filterArgsCPU fargs, struct probeArgsCPU pargs, struct groupbyArgsCPU gargs,
  int num_tuples, int* res, int start_offset = 0) {

  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(offset.h_lo_off != NULL);

  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    //int worker_index = tbb::task_arena::current_thread_index();
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;
    //printf("worker index = %d\n", worker_index);

    long long local_sum = 0;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int lo_offset;

        lo_offset = offset.h_lo_off[start_offset + i];

        // if (fargs.filter_col1 != NULL) {
          // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
          // if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
        // }

        // if (fargs.filter_col2 != NULL) {
          // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
          if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
        // }

        // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        //   if (slot == 0) continue;
        // }

        // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
        // }

        // if (gargs.mode == 0) {
        //   assert(gargs.aggr_col1 != NULL);
        //   local_sum += gargs.aggr_col1[lo_offset];
        // } else if (gargs.mode == 1) {
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
        // } else if (gargs.mode == 2){ 
        //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
          // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
        // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);
      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int lo_offset;

      lo_offset = offset.h_lo_off[start_offset + i];

      // if (fargs.filter_col1 != NULL) {
        // if (!(fargs.filter_col1[lo_offset] >= fargs.compare1 && fargs.filter_col1[lo_offset] <= fargs.compare2)) continue; //only for Q1.x
        // if (!(*(fargs.h_filter_func1))(fargs.filter_col1[lo_offset], fargs.compare1, fargs.compare2)) continue;
      // }

      // if (fargs.filter_col2 != NULL) {
        // if (!(fargs.filter_col2[lo_offset] >= fargs.compare3 && fargs.filter_col2[lo_offset] <= fargs.compare4)) continue; //only for Q1.x
        if (!(*(fargs.h_filter_func2))(fargs.filter_col2[lo_offset], fargs.compare3, fargs.compare4)) continue;
      // }

      // if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
      //   hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
      //   slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
      //   hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
      //   slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
      //   hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
      //   slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
      //   if (slot == 0) continue;
      // }

      // if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
      // }

      // if (gargs.mode == 0) {
      //   assert(gargs.aggr_col1 != NULL);
      //   local_sum += gargs.aggr_col1[lo_offset];
      // } else if (gargs.mode == 1) {
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
      //   local_sum += (gargs.aggr_col1[lo_offset] - gargs.aggr_col2[lo_offset]);
      // } else if (gargs.mode == 2){ 
      //   assert(gargs.aggr_col1 != NULL); assert(gargs.aggr_col2 != NULL);
        // local_sum += (gargs.aggr_col1[lo_offset] * gargs.aggr_col2[lo_offset]);
      // } else assert(0);

        int aggrval1 = 0, aggrval2 = 0;
        if (gargs.aggr_col1 != NULL) aggrval1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggrval2 = gargs.aggr_col2[lo_offset];
        local_sum += (*(gargs.h_group_func))(aggrval1, aggrval2);
    }

     __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[4]), (long long)(local_sum), __ATOMIC_RELAXED);

  }, simple_partitioner());

}


void merge(int* resCPU, int* resGPU, int num_tuples) {

  int grainsize = num_tuples/NUM_THREADS + 4;
  // cout << num_tuples << endl;
  // assert(grainsize < 20000);
  // assert(grainsize < SEGMENT_SIZE);

  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    int start = range.begin();
    int end = range.end();
    int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        if (resCPU[i * 6] == 0) resCPU[i * 6] = resGPU[i * 6];
        if (resCPU[i * 6 + 1] == 0) resCPU[i * 6 + 1] = resGPU[i * 6 + 1];
        if (resCPU[i * 6 + 2] == 0) resCPU[i * 6 + 2] = resGPU[i * 6 + 2];
        if (resCPU[i * 6 + 3] == 0) resCPU[i * 6 + 3] = resGPU[i * 6 + 3];
        reinterpret_cast<unsigned long long*>(resCPU)[i * 3 + 2] += reinterpret_cast<unsigned long long*>(resGPU)[i * 3 + 2];
      }
    }
    for (int i = end_batch ; i < end; i++) {
      if (resCPU[i * 6] == 0) resCPU[i * 6] = resGPU[i * 6];
      if (resCPU[i * 6 + 1] == 0) resCPU[i * 6 + 1] = resGPU[i * 6 + 1];
      if (resCPU[i * 6 + 2] == 0) resCPU[i * 6 + 2] = resGPU[i * 6 + 2];
      if (resCPU[i * 6 + 3] == 0) resCPU[i * 6 + 3] = resGPU[i * 6 + 3];
      reinterpret_cast<unsigned long long*>(resCPU)[i * 3 + 2] += reinterpret_cast<unsigned long long*>(resGPU)[i * 3 + 2];
    }
  });
}


void probe_group_by_CPU_prof(
  struct probeArgsCPU pargs,  struct groupbyArgsCPU gargs, int num_tuples, 
  int* res, int start_offset = 0, short* segment_group = NULL) {


  int grainsize = num_tuples/4096 + 4;
  assert(grainsize < 20000);
  assert(grainsize < SEGMENT_SIZE);
  assert(segment_group != NULL);


  // Probe
  parallel_for(blocked_range<size_t>(0, num_tuples, grainsize), [&](auto range) {
    unsigned int start = range.begin();
    unsigned int end = range.end();
    unsigned int end_batch = start + ((end - start)/BATCH_SIZE) * BATCH_SIZE;

    int start_segment, start_group, segment_idx;
    int end_segment;
    start_segment = segment_group[start / SEGMENT_SIZE];
    end_segment = segment_group[end / SEGMENT_SIZE];
    start_group = start / SEGMENT_SIZE;

    for (int batch_start = start; batch_start < end_batch; batch_start += BATCH_SIZE) {
      #pragma simd
      for (int i = batch_start; i < batch_start + BATCH_SIZE; i++) {
        int hash;
        long long slot;
        int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
        int lo_offset;

        if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
        else segment_idx = end_segment;

        lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

        if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
          hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
          slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
          if (slot == 0) continue;
          dim_val1 = slot;
        }



        if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
          hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
          slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
          if (slot == 0) continue;
          dim_val2 = slot;
        }

        if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
          hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
          slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
          if (slot == 0) continue;
          dim_val3 = slot;
        }

        if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
          hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
          slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
          if (slot == 0) continue;
          dim_val4 = slot;
        }

        hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
        if (dim_val1 != 0) res[hash * 6] = dim_val1;
        if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
        if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
        if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

        int aggr1 = 0; int aggr2 = 0;
        if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
        if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
        int temp = (*(gargs.h_group_func))(aggr1, aggr2);

        __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
      }
    }

    for (int i = end_batch ; i < end; i++) {

      int hash;
      long long slot;
      int dim_val1 = 0, dim_val2 = 0, dim_val3 = 0, dim_val4 = 0;
      int lo_offset;

      if ((i / SEGMENT_SIZE) == start_group) segment_idx = start_segment;
      else segment_idx = end_segment;

      lo_offset = segment_idx * SEGMENT_SIZE + (i % SEGMENT_SIZE);

      if (pargs.key_col1 != NULL && pargs.ht1 != NULL) {
        hash = HASH(pargs.key_col1[lo_offset], pargs.dim_len1, pargs.min_key1);
        slot = reinterpret_cast<long long*>(pargs.ht1)[hash];
        if (slot == 0) continue;
        dim_val1 = slot;
      }

      if (pargs.key_col2 != NULL && pargs.ht2 != NULL) {
        hash = HASH(pargs.key_col2[lo_offset], pargs.dim_len2, pargs.min_key2);
        slot = reinterpret_cast<long long*>(pargs.ht2)[hash];
        if (slot == 0) continue;
        dim_val2 = slot;
      }

      if (pargs.key_col3 != NULL && pargs.ht3 != NULL) {
        hash = HASH(pargs.key_col3[lo_offset], pargs.dim_len3, pargs.min_key3);
        slot = reinterpret_cast<long long*>(pargs.ht3)[hash];
        if (slot == 0) continue;
        dim_val3 = slot;
      }

      if (pargs.key_col4 != NULL && pargs.ht4 != NULL) {
        hash = HASH(pargs.key_col4[lo_offset], pargs.dim_len4, pargs.min_key4);
        slot = reinterpret_cast<long long*>(pargs.ht4)[hash];
        if (slot == 0) continue;
        dim_val4 = slot;
      }

      hash = ((dim_val1 - gargs.min_val1) * gargs.unique_val1 + (dim_val2 - gargs.min_val2) * gargs.unique_val2 +  (dim_val3 - gargs.min_val3) * gargs.unique_val3 + (dim_val4 - gargs.min_val4) * gargs.unique_val4) % gargs.total_val;
      if (dim_val1 != 0) res[hash * 6] = dim_val1;
      if (dim_val2 != 0) res[hash * 6 + 1] = dim_val2;
      if (dim_val3 != 0) res[hash * 6 + 2] = dim_val3;
      if (dim_val4 != 0) res[hash * 6 + 3] = dim_val4;

      int aggr1 = 0; int aggr2 = 0;
      if (gargs.aggr_col1 != NULL) aggr1 = gargs.aggr_col1[lo_offset];
      if (gargs.aggr_col2 != NULL) aggr2 = gargs.aggr_col2[lo_offset];
      int temp = (*(gargs.h_group_func))(aggr1, aggr2);

      __atomic_fetch_add(reinterpret_cast<unsigned long long*>(&res[hash * 6 + 4]), (long long)(temp), __ATOMIC_RELAXED);
    }
  }, simple_partitioner());

}
#endif