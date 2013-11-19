/*************************************************************************
> File Name: Optimizer.h
> Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
> Created Time: 2013/8/18 星期日 16:04:18
> Functions: Base class for different algorithms to do online learning
************************************************************************/

#pragma once
#include "../data/DataSet.h"
#include "../loss/LossFunction.h"
#include "../common/init_param.h"

#include <stdio.h>
#include <math.h>


/**
*  namespace: Sparse Online Learning
*/
namespace SOL {
	template <typename FeatType, typename LabelType> class Optimizer {
		//Iteration 
	protected:
		//iteration number
		size_t curIterNum;
        size_t initial_t;
        float power_t;
		//parameters
		float lambda;
		float eta0; //learning rate
        float eta;
        float eta_coeff_time; //coeff of eta with time (1/sqrt(t) for instance)

		DataSet<FeatType, LabelType> &dataSet;

		//weight vector
	protected:
		//the first element is zero
		float *weightVec;
		//weight dimenstion: can be the same to feature, or with an extra bias
		IndexType weightDim;

		//For sparse
	protected:
		float sparse_soft_thresh;

	protected:
		LossFunction<FeatType, LabelType> *lossFunc;
        
    protected:
        string id_str;

    public:
        /* by yuewuTue 15 Oct 2013 09:46:02 AM PDT*/
        /**
         * PrintOptInfo print the info of optimization algorithm
         */
        void PrintOptInfo()const {
            printf("--------------------------------------------------\n");
            printf("Algorithm: %s\n",this->Id_Str().c_str());
            printf("Learning Rate: %.2e\n", this->eta0);
            printf("Initial t  : %lu\n",this->initial_t);
            printf("Power t : %.2f\n",this->power_t); 
        }

	public:
		Optimizer(DataSet<FeatType, LabelType> &dataset, LossFunction<FeatType, LabelType> &lossFunc);

		virtual ~Optimizer() {
			if (weightVec != NULL)
				delete []this->weightVec;
		}
        const string& Id_Str() const {return this->id_str;}

	protected:
		//Reset the optimizer to the initialization status of training
		virtual void BeginTrain();
		//called when a train ends
		virtual void EndTrain();
		//train the data
		float Train();
		//predict a new feature
		float Predict(const DataPoint<FeatType, LabelType> &data);
		//predict function for test, as we are using sparse learning,dimension of the test data
        //may be larger than the model
		float Test_Predict(const DataPoint<FeatType, LabelType> &data);

		//this is the core of different updating algorithms
		//return the predict
		virtual float UpdateWeightVec(const DataPoint<FeatType, LabelType> &x) = 0;

	public:
        void SetParameter(float lambda = -1, float eta0 = -1, 
                float power_t = -1, size_t t0 = 0);
		//try and get the best parameter
		virtual void BestParameter(); 

	public:
		//learn a model
		inline float Learn(int numOfTimes = 1);
		//learn a model and return the mistake rate and its variance
		float Learn(float &aveErrRate, float &varErrRate, float &sparseRate, int numOfTimes = 1);
		//test the performance on the given set
		float Test(DataSet<FeatType, LabelType> &testSet);

		float GetSparseRate(IndexType total_len = 0);

	protected:
		//Change the dimension of weights
		virtual void UpdateWeightSize(IndexType newDim);

    protected:
        float (*pEta_time)(size_t t, float pt);
    };
    
    //calculate learning rate
    inline float pEta_general(size_t t, float pt){
        return std::pow(t,pt);
    }
    inline float pEta_sqrt(size_t t, float pt){
        return std::sqrt(t);
    }
    inline float pEta_linear(size_t t, float pt){
        return t;
    }
    inline float pEta_const(size_t t, float pt){
        return 1;
    }

    template <typename FeatType, typename LabelType>
        Optimizer<FeatType, LabelType>::Optimizer(DataSet<FeatType, LabelType> &dataset, 
                LossFunction<FeatType, LabelType> &lossFunc): dataSet(dataset) {
            this->lossFunc = &lossFunc;
            this->weightDim = 1;
            //weight vector
            this->weightVec = new float[this->weightDim];

            this->eta0 = init_eta;;
            this->lambda = init_lambda;
            this->curIterNum = 0;
            this->initial_t = init_initial_t;
            this->power_t = init_power_t;

            this->sparse_soft_thresh = init_sparse_soft_thresh;
        }

    //reset the optimizer to this initialization
    template <typename FeatType, typename LabelType>
        void Optimizer<FeatType, LabelType>::BeginTrain() {
            //reset weight vector
            memset(this->weightVec,0,sizeof(float ) * this->weightDim);
            this->curIterNum = this->initial_t;

            if (this->power_t == 0.5)
                this->pEta_time = pEta_sqrt;
            else if(this->power_t == 0)
                this->pEta_time = pEta_const;
            else if (this->power_t == 1)
                this->pEta_time = pEta_linear;
            else
                this->pEta_time = pEta_general;
        }

    //called when a train ends
    template <typename FeatType, typename LabelType>
        void Optimizer<FeatType, LabelType>::EndTrain() {
            for (IndexType i = 1; i < this->weightDim; i++){
                if (this->weightVec[i] < this->sparse_soft_thresh && 
                        this->weightVec[i] > -this->sparse_soft_thresh ){
                    this->weightVec[i] = 0;
                }
            }
        }

    template <typename FeatType, typename LabelType> 
        float Optimizer<FeatType, LabelType>::Train() {
            if(dataSet.Rewind() == false)
                exit(0);
            //reset
            this->BeginTrain();
            float errorNum(0);
            size_t show_step = 1; //show information every show_step
            size_t show_count = 3;

            printf("Iterate No.\t\tError Rate\t\t\n");
            while(1) {
                const DataChunk<FeatType,LabelType> &chunk = dataSet.GetChunk();
                //all the data has been processed!
                if(chunk.dataNum  == 0) 
                    break;

                for (size_t i = 0; i < chunk.dataNum; i++) {

                    this->eta_coeff_time = this->pEta_time(this->curIterNum, this->power_t);
                    this->eta = this->eta0 / this->eta_coeff_time;

                    const DataPoint<FeatType, LabelType> &data = chunk.data[i];
                    this->UpdateWeightSize(data.dim());
                    float y = this->UpdateWeightVec(data); 
                    //loss
                    if (this->lossFunc->IsCorrect(data.label,y) == false)
                        errorNum++;
                    show_count--;
                    if (show_count == 0){
                        printf("%lu\t\t\t%.6f\t\t\n",this->curIterNum, 
                                errorNum / (float)(this->curIterNum));
                        show_step *= 2;
                        show_count = show_step;
                    }
                    this->curIterNum++;
                }
                dataSet.FinishRead();
            }
            this->EndTrain();

            return errorNum / dataSet.size();
        }

    //learn a model and return the mistake rate and its variance
    template <typename FeatType, typename LabelType>
        float Optimizer<FeatType, LabelType>::Learn(float &aveErrRate, float &varErrRate, 
                float &sparseRate, int numOfTimes) {
            float * errorRateVec = new float[numOfTimes];
            float * sparseRateVec = new float[numOfTimes];

            for (int i = 0; i < numOfTimes; i++) {
                //random order
                errorRateVec[i] = this->Train();
                sparseRateVec[i] = this->GetSparseRate();
            }
            aveErrRate = Average(errorRateVec, numOfTimes);
            varErrRate = Variance(errorRateVec, numOfTimes);
            sparseRate = Average(sparseRateVec, numOfTimes);

            delete []errorRateVec;
            delete []sparseRateVec;

            return aveErrRate;
        }

    //learn a model
    template <typename FeatType, typename LabelType>
        float Optimizer<FeatType, LabelType>::Learn(int numOfTimes) {
            float aveErrRate, varErrRate, sparseRate;
            return this->Learn(aveErrRate, varErrRate,sparseRate, numOfTimes);
        }

    //test the performance on the given set
    template <typename FeatType, typename LabelType>
        float Optimizer<FeatType, LabelType>::Test(DataSet<FeatType, LabelType> &testSet) {
            if(testSet.Rewind() == false)
                exit(0);
            float errorRate(0);
            //test
            while(1) {
                const DataChunk<FeatType,LabelType> &chunk = testSet.GetChunk();
                if(chunk.dataNum  == 0) //"all the data has been processed!"
                    break;
                for (size_t i = 0; i < chunk.dataNum; i++) {
                    const DataPoint<FeatType , LabelType> &data = chunk.data[i];
                    //predict
                    float predict = this->Test_Predict(data);
                    if (this->lossFunc->IsCorrect(data.label,predict) == false)
                        errorRate++;
                }
                testSet.FinishRead();
            }
            errorRate /= testSet.size();
            return errorRate;
        }

    template <typename FeatType, typename LabelType>
        float Optimizer<FeatType, LabelType>::Test_Predict(const DataPoint<FeatType, LabelType> &data) {
            float predict = 0;
            int dim = data.indexes.size();
            for (int i = 0; i < dim; i++){
                if (data.indexes[i] < this->weightDim)
                    predict += this->weightVec[data.indexes[i]] * data.features[i];
            }
            predict += this->weightVec[0];
            return predict;
        }
    template <typename FeatType, typename LabelType>
        float Optimizer<FeatType, LabelType>::Predict(const DataPoint<FeatType, LabelType> &data) {
            float predict = 0;
            size_t dim = data.indexes.size();
            for (size_t i = 0; i < dim; i++){
                predict += this->weightVec[data.indexes[i]] * data.features[i];
            }
            predict += this->weightVec[0];
            return predict;
        }


    template <typename FeatType, typename LabelType>
        float Optimizer<FeatType, LabelType>::GetSparseRate(IndexType total_len) {
            float zeroNum(0);
            if (this->weightDim == 1)
                return 1;

            for (IndexType i = 1; i < this->weightDim; i++) {
                if (this->weightVec[i] == 0)
                    zeroNum++;
            }
            if (total_len > 0)
                return zeroNum / total_len;
            else
                return zeroNum / (this->weightDim - 1);
        }

    //try and get the best parameter
    template <typename FeatType, typename LabelType>
        void Optimizer<FeatType, LabelType>::BestParameter() {
            float prev_lambda = this->lambda;
            this->lambda = 0;
            //1. Select the best eta0

            float min_errorRate = 1;
            float bestEta = 1;

            for (float eta_c = init_eta_min; eta_c<= init_eta_max; eta_c *= init_eta_step) {
                cout<<"eta0 = "<<eta_c<<"\n";
                float errorRate(0);
                this->eta0 = eta_c;
                errorRate += this->Train();

                if (errorRate < min_errorRate) {
                    bestEta = eta_c;
                    min_errorRate = errorRate;
                }
                cout<<"mistake rate: "<<errorRate * 100<<" %\n";
            }
            this->eta0 = bestEta;
            this->lambda = prev_lambda;
            cout<<"Best Parameter:\teta = "<<this->eta0<<"\n\n";
        }

    template <typename FeatType, typename LabelType>
        void Optimizer<FeatType, LabelType>::SetParameter(float lambda , float eta0, 
                float power_t,  size_t t0 ){
            this->lambda  = lambda >= 0 ? lambda : this->lambda;
            this->eta0 = eta0 > 0 ? eta0 : this->eta0;
            this->power_t = power_t >= 0 ? power_t : this->power_t;
            this->initial_t = t0 > 0 ? t0: this->initial_t;
        }

    //Change the dimension of weights
    template <typename FeatType, typename LabelType>
        void Optimizer<FeatType, LabelType>::UpdateWeightSize(IndexType newDim) {
            if (newDim < this->weightDim) 
                return;
            else {
                newDim++; //reserve the 0-th
                float* newW = new float[newDim];
                memset(newW,0,sizeof(float) * newDim); 

                //copy info
                memcpy(newW,this->weightVec,sizeof(float) * this->weightDim); 
                //set the rest to zero
                memset(newW + this->weightDim,0,sizeof(float) * (newDim - this->weightDim));

                delete []this->weightVec;
                this->weightVec = newW;
                this->weightDim = newDim;
            }
        }
}
