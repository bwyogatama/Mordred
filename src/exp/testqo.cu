#include "QueryProcessing2.h"

#include <chrono>
#include <atomic>
#include <string>

// tbb::task_scheduler_init init(1);

int main() {

	bool verbose = 0;

	srand(123);
	
	CPUGPUProcessing* cgp = new CPUGPUProcessing(209715200 * 2, 209715200, 536870912, 536870912, 0);

	cout << "Profiling" << endl;
	QueryProcessing* qp = new QueryProcessing(cgp, 11, 0);
	qp->profile();

	cgp->cm->resetCache(209715200, 209715200 / 2, 536870912, 536870912);

	cout << endl;

	// CacheManager* cm = cgp->cm;

	// cm->cacheColumnSegmentInGPU(cm->lo_orderdate, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_suppkey, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_custkey, 58);
	// cm->cacheColumnSegmentInGPU(cm->lo_partkey, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_revenue, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_supplycost, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_discount, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_quantity, 0);
	// cm->cacheColumnSegmentInGPU(cm->lo_extendedprice, 0);
	// cm->cacheColumnSegmentInGPU(cm->d_datekey, cm->d_datekey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->d_year, 0);
	// cm->cacheColumnSegmentInGPU(cm->d_yearmonthnum, 0);
	// cm->cacheColumnSegmentInGPU(cm->p_partkey, cm->p_partkey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->p_category, 0);
	// cm->cacheColumnSegmentInGPU(cm->p_brand1, 0);
	// cm->cacheColumnSegmentInGPU(cm->p_mfgr, 0);
	// cm->cacheColumnSegmentInGPU(cm->c_custkey, cm->c_custkey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->c_region, 0);
	// cm->cacheColumnSegmentInGPU(cm->c_nation, 0);
	// cm->cacheColumnSegmentInGPU(cm->c_city, 0);
	// cm->cacheColumnSegmentInGPU(cm->s_suppkey, cm->s_suppkey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->s_region, 0);
	// cm->cacheColumnSegmentInGPU(cm->s_nation, 0);
	// cm->cacheColumnSegmentInGPU(cm->s_city, 0);

	// cm->cacheColumnSegmentInGPU(cm->d_datekey, cm->d_datekey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->d_year, cm->d_year->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->d_yearmonthnum, cm->d_yearmonthnum->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->p_partkey, cm->p_partkey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->p_category, cm->p_category->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->p_brand1, cm->p_brand1->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->p_mfgr, cm->p_mfgr->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->c_custkey, cm->c_custkey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->c_region, cm->c_region->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->c_nation, cm->c_nation->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->c_city, cm->c_city->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->s_suppkey, cm->s_suppkey->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->s_region, cm->s_region->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->s_nation, cm->s_nation->total_segment);
	// cm->cacheColumnSegmentInGPU(cm->s_city, cm->s_city->total_segment);

	bool exit = 0;
	string input;
	string query;
	double time = 0;
	double time1 = 0, time2 = 0;

	while (!exit) {
		cout << "Select Options:" << endl;
		cout << "1. Run Specific Query" << endl;
		cout << "2. Run Random Queries" << endl;
		cout << "3. Update Cache (LFU)" << endl;
		cout << "4. Update Cache (LRU)" << endl;
		cout << "5. Update Cache (New)" << endl;
		cout << "6. Update Cache (New+)" << endl;
		cout << "7. Profiling" << endl;
		cout << "8. Exit" << endl;
		cout << "Your Input: ";
		cin >> input;

		if (input.compare("1") == 0) {
			cout << "Input Query: ";
			cin >> query;
			QueryProcessing* qp = new QueryProcessing(cgp, stoi(query), verbose);
			qp->processQuery();
			time1 = qp->processQuery();
			qp->processQuery2();
			time2 = qp->processQuery2();
			if (time1 <= time2) time += time1;
			else time += time2;
			delete qp;
		} else if (input.compare("2") == 0) {
			time = 0;
			cout << "Executing Random Query" << endl;
			QueryProcessing* qp = new QueryProcessing(cgp, 11, verbose);
			for (int i = 0; i < 100; i++) {
				qp->generate_rand_query();
				time1 = qp->processQuery();
				time2 = qp->processQuery2();
				if (time1 <= time2) time += time1;
				else time += time2;
			}
			delete qp;
		} else if (input.compare("3") == 0) {
			cout << "LFU Replacement" << endl;
			cgp->cm->runReplacement(LFU);
			QueryProcessing* qp = new QueryProcessing(cgp, 11, verbose);
			qp->percentageData();
			delete qp;
			time = 0;
			srand(123);
		} else if (input.compare("4") == 0) {
			cout << "LRU Replacement" << endl;
			cgp->cm->runReplacement(LRU);
			QueryProcessing* qp = new QueryProcessing(cgp, 11, verbose);
			qp->percentageData();
			delete qp;
			time = 0;
			srand(123);
		} else if (input.compare("5") == 0) {
			cout << "New Replacement" << endl;
			cgp->cm->runReplacement(New);
			QueryProcessing* qp = new QueryProcessing(cgp, 11, verbose);
			qp->percentageData();
			delete qp;
			time = 0;
			srand(123);
		} else if (input.compare("6") == 0) {
			cout << "New+ Replacement" << endl;
			cgp->cm->runReplacement(New_v2);
			QueryProcessing* qp = new QueryProcessing(cgp, 11, verbose);
			qp->percentageData();
			delete qp;
			time = 0;
			srand(123);
		} else if (input.compare("7") == 0) {
			cout << "Profiling" << endl;
			QueryProcessing* qp = new QueryProcessing(cgp, 11, verbose);
			qp->profile();
			delete qp;
		} else {
			exit = true;
		}

		cout << endl;
		cout << "Cumulated Time: " << time << endl;
		cout << endl;

	}

}