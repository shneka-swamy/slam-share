/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2020 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
* the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with ORB-SLAM3.
* If not, see <http://www.gnu.org/licenses/>.
*/

#include "MapPoint.h"
#include "ORBmatcher.h"
#include "System.h"

#include<mutex>

namespace ORB_SLAM3
{

long unsigned int MapPoint::nNextId=0;
mutex MapPoint::mGlobalMutex;

MapPoint::MapPoint():
    mnFirstKFid(0), mnFirstFrame(0), nObs(0), mnTrackReferenceForFrame(0),
    mnLastFrameSeen(0), mnBALocalForKF(0), mnFuseCandidateForKF(0), mnLoopPointForKF(0), mnCorrectedByKF(0),
    mnCorrectedReference(0), mnBAGlobalForKF(0), mnVisible(1), mnFound(1), mbBad(false),
    mpReplaced(static_cast<boost::interprocess::offset_ptr<MapPoint> >(NULL))
{
    mpReplaced = static_cast<boost::interprocess::offset_ptr<MapPoint> >(NULL);
}

MapPoint::MapPoint(const cv::Mat &Pos, boost::interprocess::offset_ptr<KeyFrame> pRefKF, boost::interprocess::offset_ptr<Map>  pMap):
    mnFirstKFid(pRefKF->mnId), mnFirstFrame(pRefKF->mnFrameId), nObs(0), mnTrackReferenceForFrame(0),
    mnLastFrameSeen(0), mnBALocalForKF(0), mnFuseCandidateForKF(0), mnLoopPointForKF(0), mnCorrectedByKF(0),
    mnCorrectedReference(0), mnBAGlobalForKF(0), mpRefKF(pRefKF), mnVisible(1), mnFound(1), mbBad(false),
    mpReplaced(static_cast<boost::interprocess::offset_ptr<MapPoint> >(NULL)), mfMinDistance(0), mfMaxDistance(0), mpMap(pMap),
    mnOriginMapId(pMap->GetId())
{
    //initialize data for cv matrix mNormalVector
    mWorldPos_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mWorldPos = cv::Mat(3,1,CV_32F,mWorldPos_ptr.get());
    //std::cout<<"Shared memory data for worldpos "<<mWorldPos_ptr<<std::endl;

    Pos.copyTo(mWorldPos);
    mWorldPosx = cv::Matx31f(Pos.at<float>(0), Pos.at<float>(1), Pos.at<float>(2));
    
    //initialize data for cv matrix mNormalVector
    mNormalVector_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    memset(mNormalVector_ptr.get(), 0, 3*4);//zeroing the mNormalVector
    mNormalVector = cv::Mat(3,1,CV_32F,mNormalVector_ptr.get());

    mNormalVectorMerge_ptr = ORB_SLAM3::allocator_instance.allocate(1*3*4);
    mNormalVectorMerge = cv::Mat(1,3,CV_32F,mNormalVectorMerge_ptr.get());



    //initialize data for cv matrix mDescriptor
    mDescriptor_ptr = ORB_SLAM3::allocator_instance.allocate(32*1*4);
    mDescriptor = cv::Mat(32,1,CV_32F,mDescriptor_ptr.get());

    mPosMerge_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mPosMerge = cv::Mat(1,3,CV_32F,mPosMerge_ptr.get());

    //mposgba
    mPosGBA_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mPosGBA = cv::Mat(3,1,CV_32F,mPosGBA_ptr.get());

    //mNormalVector_ptr = mNormalVector_data;
    //mNormalVector = cv::Mat::zeros(3,1,CV_32F);
    mNormalVectorx = cv::Matx31f::zeros();

    mbTrackInViewR = false;
    mbTrackInView = false;

    // MapPoints can be created from Tracking and Local Mapping. This mutex avoid conflicts with id.
    std::scoped_lock<mutex> lock(mpMap->mMutexPointCreation);
    mnId=nNextId++;

    //the observations
    const ShmemAllocator_observation alloc_map_observe(ORB_SLAM3::segment.get_segment_manager());
    mObservations = ORB_SLAM3::segment.construct<Observe_map>(boost::interprocess::anonymous_instance)(alloc_map_observe);
}

MapPoint::MapPoint(const double invDepth, cv::Point2f uv_init, boost::interprocess::offset_ptr<KeyFrame>  pRefKF, boost::interprocess::offset_ptr<KeyFrame>  pHostKF, boost::interprocess::offset_ptr<Map>  pMap):
    mnFirstKFid(pRefKF->mnId), mnFirstFrame(pRefKF->mnFrameId), nObs(0), mnTrackReferenceForFrame(0),
    mnLastFrameSeen(0), mnBALocalForKF(0), mnFuseCandidateForKF(0), mnLoopPointForKF(0), mnCorrectedByKF(0),
    mnCorrectedReference(0), mnBAGlobalForKF(0), mpRefKF(pRefKF), mnVisible(1), mnFound(1), mbBad(false),
    mpReplaced(static_cast<boost::interprocess::offset_ptr<MapPoint> >(NULL)), mfMinDistance(0), mfMaxDistance(0), mpMap(pMap),
    mnOriginMapId(pMap->GetId())
{
    mInvDepth=invDepth;
    mInitU=(double)uv_init.x;
    mInitV=(double)uv_init.y;
    mpHostKF = pHostKF;

    std::cout<<"2nd *** Shared memory data for worldpos "<<mWorldPos_ptr<<std::endl;

    //initialize data for cv matrix mNormalVector
    mNormalVector_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    memset(mNormalVector_ptr.get(), 0, 3*4);//zeroing the mNormalVector
    mNormalVector = cv::Mat(3,1,CV_32F,mNormalVector_ptr.get());

    mNormalVectorMerge_ptr = ORB_SLAM3::allocator_instance.allocate(1*3*4);
    mNormalVectorMerge = cv::Mat(1,3,CV_32F,mNormalVectorMerge_ptr.get());


    //initialize data for cv matrix mDescriptor
    mDescriptor_ptr = ORB_SLAM3::allocator_instance.allocate(32*1*4);
    mDescriptor = cv::Mat(32,1,CV_32F,mDescriptor_ptr.get());

     //mposgba
    mPosGBA_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mPosGBA = cv::Mat(3,1,CV_32F,mPosGBA_ptr.get());

    mPosMerge_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mPosMerge = cv::Mat(1,3,CV_32F,mPosMerge_ptr.get());

    //mNormalVector_ptr = mNormalVector_data;
    //mNormalVector = cv::Mat::zeros(3,1,CV_32F);
    mNormalVectorx = cv::Matx31f::zeros();

    // Worldpos is not set
    // MapPoints can be created from Tracking and Local Mapping. This mutex avoid conflicts with id.
    std::scoped_lock<mutex> lock(mpMap->mMutexPointCreation);
    mnId=nNextId++;

    //the observations
    const ShmemAllocator_observation alloc_map_observe(ORB_SLAM3::segment.get_segment_manager());
    mObservations = ORB_SLAM3::segment.construct<Observe_map>(boost::interprocess::anonymous_instance)(alloc_map_observe);
}

MapPoint::MapPoint(const cv::Mat &Pos, boost::interprocess::offset_ptr<Map>  pMap, Frame* pFrame, const int &idxF):
    mnFirstKFid(-1), mnFirstFrame(pFrame->mnId), nObs(0), mnTrackReferenceForFrame(0), mnLastFrameSeen(0),
    mnBALocalForKF(0), mnFuseCandidateForKF(0),mnLoopPointForKF(0), mnCorrectedByKF(0),
    mnCorrectedReference(0), mnBAGlobalForKF(0), mpRefKF(static_cast<boost::interprocess::offset_ptr<KeyFrame> >(NULL)), mnVisible(1),
    mnFound(1), mbBad(false), mpReplaced(NULL), mpMap(pMap), mnOriginMapId(pMap->GetId())
{

    //initialize data for cv matrix mNormalVector
    mNormalVector_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    memset(mNormalVector_ptr.get(), 0, 3*4);//zeroing the mNormalVector
    mNormalVector = cv::Mat(3,1,CV_32F,mNormalVector_ptr.get());

    //initialize all the cv matrix here:
    // get size from above.
    //void *mworldPos_data = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    // create mworldpos data

    //initialize data for cv matrix mDescriptor
    mDescriptor_ptr = ORB_SLAM3::allocator_instance.allocate(32*1*4);
    mDescriptor = cv::Mat(32,1,CV_32F,mDescriptor_ptr.get());
    
    //initialize data for cv matrix mNormalVector
    mWorldPos_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mWorldPos = cv::Mat(3,1,CV_32F,mWorldPos_ptr.get());

    //mposgba
    mPosGBA_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mPosGBA = cv::Mat(3,1,CV_32F,mPosGBA_ptr.get());

    mPosMerge_ptr = ORB_SLAM3::allocator_instance.allocate(3*1*4);
    mPosMerge = cv::Mat(1,3,CV_32F,mPosMerge_ptr.get());

     mNormalVectorMerge_ptr = ORB_SLAM3::allocator_instance.allocate(1*3*4);
    mNormalVectorMerge = cv::Mat(1,3,CV_32F,mNormalVectorMerge_ptr.get());

    //std::cout<<"Shared memory data for worldpos "<<mWorldPos_ptr<<std::endl;

    Pos.copyTo(mWorldPos);
    mWorldPosx = cv::Matx31f(Pos.at<float>(0), Pos.at<float>(1), Pos.at<float>(2));

    cv::Mat Ow;
    if(pFrame -> Nleft == -1 || idxF < pFrame -> Nleft){
        Ow = pFrame->GetCameraCenter();
    }
    else{
        cv::Mat Rwl = pFrame -> mRwc;
        cv::Mat tlr = pFrame -> mTlr.col(3);
        cv::Mat twl = pFrame -> mOw;

        Ow = Rwl * tlr + twl;
    }
    cv::Mat normaltemp =  mWorldPos - Ow;
    normaltemp = normaltemp/cv::norm(normaltemp);
    //mNormalVector = mWorldPos - Ow;
    //mNormalVector = mNormalVector/cv::norm(mNormalVector);
    normaltemp.copyTo(mNormalVector);
    mNormalVectorx = cv::Matx31f(mNormalVector.at<float>(0), mNormalVector.at<float>(1), mNormalVector.at<float>(2));


    cv::Mat PC = Pos - Ow;
    const float dist = cv::norm(PC);
    const int level = (pFrame -> Nleft == -1) ? pFrame->mvKeysUn[idxF].octave
                                              : (idxF < pFrame -> Nleft) ? pFrame->mvKeys[idxF].octave
                                                                         : pFrame -> mvKeysRight[idxF].octave;
    const float levelScaleFactor =  pFrame->mvScaleFactors[level];
    const int nLevels = pFrame->mnScaleLevels;

    mfMaxDistance = dist*levelScaleFactor;
    mfMinDistance = mfMaxDistance/pFrame->mvScaleFactors[nLevels-1];

    pFrame->mDescriptors.row(idxF).copyTo(mDescriptor);

    // MapPoints can be created from Tracking and Local Mapping. This mutex avoid conflicts with id.
    std::scoped_lock<mutex> lock(mpMap->mMutexPointCreation);
    mnId=nNextId++;

    //the observations
    const ShmemAllocator_observation alloc_map_observe(ORB_SLAM3::segment.get_segment_manager());
    mObservations = ORB_SLAM3::segment.construct<Observe_map>(boost::interprocess::anonymous_instance)(alloc_map_observe);
}

void MapPoint::SetWorldPos(const cv::Mat &Pos)
{
    std::scoped_lock<mutex> lock2(mGlobalMutex);
    std::scoped_lock<mutex> lock(mMutexPos);
    Pos.copyTo(mWorldPos);
    mWorldPosx = cv::Matx31f(Pos.at<float>(0), Pos.at<float>(1), Pos.at<float>(2));
}

void MapPoint::FixMatrices(){
    cv::Mat *fake = new cv::Mat(3,1,CV_32F,mWorldPos_ptr.get());
    memcpy(&mWorldPos, fake, sizeof(cv::Mat));

    cv::Mat *fake1 = new cv::Mat(32,1,CV_32F,mDescriptor_ptr.get());
    memcpy(&mDescriptor, fake1, sizeof(cv::Mat));

    cv::Mat *fake2 = new cv::Mat(3,1,CV_32F,mNormalVector_ptr.get());
    memcpy(&mNormalVector,fake2, sizeof(cv::Mat));

    cv::Mat *fake3 = new cv::Mat(3,1,CV_32F,mPosGBA_ptr.get());
    memcpy(&mPosGBA, fake3, sizeof(cv::Mat));

    cv::Mat *fake4 = new cv::Mat(1,3,CV_32F,mPosMerge_ptr.get());
    memcpy(&mPosMerge, fake4, sizeof(cv::Mat));

    cv::Mat *fake5 = new cv::Mat(1,3,CV_32F,mNormalVectorMerge_ptr.get());
    memcpy(&mNormalVectorMerge, fake5, sizeof(cv::Mat));



    //now correct mWorldpos with original data
}

cv::Mat MapPoint::GetWorldPos()
{
    std::scoped_lock<mutex> lock(mMutexPos);
    cv::Mat returnable = cv::Mat(3,1,CV_32F,mWorldPos_ptr.get());
    return returnable.clone();
    //return mWorldPos.clone();
}

cv::Mat MapPoint::GetNormal()
{
    std::scoped_lock<mutex> lock(mMutexPos);
    cv::Mat temp = cv::Mat(3,1,CV_32F,mNormalVector_ptr.get());
    return temp.clone();
    //return mNormalVector.clone();
}

cv::Matx31f MapPoint::GetWorldPos2()
{
    std::scoped_lock<mutex> lock(mMutexPos);
    return mWorldPosx;
}

cv::Matx31f MapPoint::GetNormal2()
{
    std::scoped_lock<mutex> lock(mMutexPos);
    return mNormalVectorx;
}

boost::interprocess::offset_ptr<KeyFrame>  MapPoint::GetReferenceKeyFrame()
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    return mpRefKF;
}

void MapPoint::AddObservation(boost::interprocess::offset_ptr<KeyFrame>  pKF, int idx)
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    tuple<int,int> indexes;

    //if(mObservations.count(pKF)){
    if(mObservations->count(pKF)){
        //indexes = mObservations[pKF];
        //indexes = mObservations->at(pKF);
        indexes = (*mObservations)[pKF];
    }
    else{
        indexes = tuple<int,int>(-1,-1);
    }
    //std::cout<<"AddObservation1.\n";

    if(pKF -> NLeft != -1 && idx >= pKF -> NLeft){
        get<1>(indexes) = idx;
    }
    else{
        get<0>(indexes) = idx;
    }

    //std::cout<<"AddObservation2.\n";
    //mObservations[pKF]=indexes;
    (*mObservations)[pKF] = indexes;
    //std::cout<<"AddObservation3.\n";
    if(!pKF->mpCamera2 && pKF->mvuRight->at(idx)>=0)//if(!pKF->mpCamera2 && pKF->mvuRight[idx]>=0)
        nObs+=2;
    else
        nObs++;
    //std::cout<<"AddObservation4.\n";
}

void MapPoint::EraseObservation(boost::interprocess::offset_ptr<KeyFrame>  pKF)
{
    bool bBad=false;
    {
        std::_lock<mutex> lock(mMutexFeatures);
        //if(mObservations.count(pKF))
        if(mObservations->count(pKF))
        {
            //tuple<int,int> indexes = mObservations[pKF];
            tuple<int,int> indexes = (*mObservations)[pKF];
            int leftIndex = get<0>(indexes), rightIndex = get<1>(indexes);

            if(leftIndex != -1){
                if(!pKF->mpCamera2 && pKF->mvuRight->at(leftIndex)>=0)//if(!pKF->mpCamera2 && pKF->mvuRight[leftIndex]>=0)
                    nObs-=2;
                else
                    nObs--;
            }
            if(rightIndex != -1){
                nObs--;
            }

            //mObservations.erase(pKF);
            mObservations->erase(pKF);

            if(mpRefKF==pKF){
                //mpRefKF=mObservations.begin()->first;
                mpRefKF=mObservations->begin()->first;
            }

            // If only 2 observations or less, discard point
            if(nObs<=2)
                bBad=true;
        }
    }

    if(bBad)
        SetBadFlag();
}


std::map<boost::interprocess::offset_ptr<KeyFrame> , std::tuple<int,int>>  MapPoint::GetObservations()
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    std::map<boost::interprocess::offset_ptr<KeyFrame> , std::tuple<int,int> > returnable;

    //for (auto const& x: mObservations){
    //    returnable.insert(x);
    //}
    
    returnable.insert(mObservations->begin(), mObservations->end());
    //return mObservations;
    return returnable;
}

int MapPoint::Observations()
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    return nObs;
}

void MapPoint::SetBadFlag()
{
    map<boost::interprocess::offset_ptr<KeyFrame> , tuple<int,int>> obs;
    {
        std::scoped_lock<mutex> lock1(mMutexFeatures);
        std::scoped_lock<mutex> lock2(mMutexPos);
        mbBad=true;
        obs.insert(mObservations->begin(), mObservations->end());
        //obs = mObservations;
        //mObservations.clear();
        mObservations->clear();
    }
    for(map<boost::interprocess::offset_ptr<KeyFrame> , tuple<int,int>>::iterator mit=obs.begin(), mend=obs.end(); mit!=mend; mit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKF = mit->first;
        int leftIndex = get<0>(mit -> second), rightIndex = get<1>(mit -> second);
        if(leftIndex != -1){
            pKF->EraseMapPointMatch(leftIndex);
        }
        if(rightIndex != -1){
            pKF->EraseMapPointMatch(rightIndex);
        }
    }

    mpMap->EraseMapPoint(this);
}

boost::interprocess::offset_ptr<MapPoint>  MapPoint::GetReplaced()
{
    std::scoped_lock<mutex> lock1(mMutexFeatures);
    std::scoped_lock<mutex> lock2(mMutexPos);
    return mpReplaced;
}

void MapPoint::Replace(boost::interprocess::offset_ptr<MapPoint>  pMP)
{
    if(pMP->mnId==this->mnId)
        return;

    int nvisible, nfound;
    map<boost::interprocess::offset_ptr<KeyFrame> ,tuple<int,int>> obs;
    {
        std::scoped_lock<mutex> lock1(mMutexFeatures);
        std::scoped_lock<mutex> lock2(mMutexPos);
        obs.insert(mObservations->begin(), mObservations->end());
        mObservations->clear();
        //obs=mObservations;
        //mObservations.clear();
        mbBad=true;
        nvisible = mnVisible;
        nfound = mnFound;
        mpReplaced = pMP;
    }

    for(map<boost::interprocess::offset_ptr<KeyFrame> ,tuple<int,int>>::iterator mit=obs.begin(), mend=obs.end(); mit!=mend; mit++)
    {
        // Replace measurement in keyframe
        boost::interprocess::offset_ptr<KeyFrame>  pKF = mit->first;

        tuple<int,int> indexes = mit -> second;
        int leftIndex = get<0>(indexes), rightIndex = get<1>(indexes);

        if(!pMP->IsInKeyFrame(pKF))
        {
            if(leftIndex != -1){
                pKF->ReplaceMapPointMatch(leftIndex, pMP);
                pMP->AddObservation(pKF,leftIndex);
            }
            if(rightIndex != -1){
                pKF->ReplaceMapPointMatch(rightIndex, pMP);
                pMP->AddObservation(pKF,rightIndex);
            }
        }
        else
        {
            if(leftIndex != -1){
                pKF->EraseMapPointMatch(leftIndex);
            }
            if(rightIndex != -1){
                pKF->EraseMapPointMatch(rightIndex);
            }
        }
    }
    pMP->IncreaseFound(nfound);
    pMP->IncreaseVisible(nvisible);
    pMP->ComputeDistinctiveDescriptors();

    mpMap->EraseMapPoint(this);
}

bool MapPoint::isBad()
{
    // std::scoped_lock<mutex> lock1(mMutexFeatures,std::defer_lock);
    // std::scoped_lock<mutex> lock2(mMutexPos,std::defer_lock);
    std::scoped_lock<mutex, mutex> lock(mMutexFeatures, mMutexPos);
    //std::scoped_lock<mutex> lock2(mMutexPos);
    //lock(lock1, lock2);

    return mbBad;
}

void MapPoint::IncreaseVisible(int n)
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    mnVisible+=n;
}

void MapPoint::IncreaseFound(int n)
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    mnFound+=n;
}

float MapPoint::GetFoundRatio()
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    return static_cast<float>(mnFound)/mnVisible;
}

void MapPoint::ComputeDistinctiveDescriptors()
{
    // Retrieve all observed descriptors
    vector<cv::Mat> vDescriptors;

    map<boost::interprocess::offset_ptr<KeyFrame> ,tuple<int,int>> observations;

    {
        std::scoped_lock<mutex> lock1(mMutexFeatures);
        if(mbBad)
            return;
        //observations=mObservations;
        observations.insert(mObservations->begin(), mObservations->end());
    }

    if(observations.empty())
        return;

    vDescriptors.reserve(observations.size());

    for(map<boost::interprocess::offset_ptr<KeyFrame> ,tuple<int,int>>::iterator mit=observations.begin(), mend=observations.end(); mit!=mend; mit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKF = mit->first;

        if(!pKF->isBad()){
            tuple<int,int> indexes = mit -> second;
            int leftIndex = get<0>(indexes), rightIndex = get<1>(indexes);

            if(leftIndex != -1){
                vDescriptors.push_back(pKF->mDescriptors.row(leftIndex));
            }
            if(rightIndex != -1){
                vDescriptors.push_back(pKF->mDescriptors.row(rightIndex));
            }
        }
    }

    if(vDescriptors.empty())
        return;

    // Compute distances between them
    const size_t N = vDescriptors.size();

    float Distances[N][N];
    for(size_t i=0;i<N;i++)
    {
        Distances[i][i]=0;
        for(size_t j=i+1;j<N;j++)
        {
            int distij = ORBmatcher::DescriptorDistance(vDescriptors[i],vDescriptors[j]);
            Distances[i][j]=distij;
            Distances[j][i]=distij;
        }
    }

    // Take the descriptor with least median distance to the rest
    int BestMedian = INT_MAX;
    int BestIdx = 0;
    for(size_t i=0;i<N;i++)
    {
        vector<int> vDists(Distances[i],Distances[i]+N);
        sort(vDists.begin(),vDists.end());
        int median = vDists[0.5*(N-1)];

        if(median<BestMedian)
        {
            BestMedian = median;
            BestIdx = i;
        }
    }

    {
        std::scoped_lock<mutex> lock(mMutexFeatures);
        //mDescriptor = vDescriptors[BestIdx].clone();
        //std::cout<<"vDescriptors[BestIdx] size: "<<vDescriptors[BestIdx].size()<<" and size of mDescriptor "<<mDescriptor.size()<<std::endl;
        vDescriptors[BestIdx].copyTo(mDescriptor);
    }
}

cv::Mat MapPoint::GetDescriptor()
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    return mDescriptor.clone();
}

tuple<int,int> MapPoint::GetIndexInKeyFrame(boost::interprocess::offset_ptr<KeyFrame> pKF)
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    //if(mObservations.count(pKF))
    //    return mObservations[pKF];
    if(mObservations->count(pKF))
        return (*mObservations)[pKF];
    else
        return tuple<int,int>(-1,-1);
}

bool MapPoint::IsInKeyFrame(boost::interprocess::offset_ptr<KeyFrame> pKF)
{
    std::scoped_lock<mutex> lock(mMutexFeatures);
    //return (mObservations.count(pKF));
    return (mObservations->count(pKF));
}

void MapPoint::UpdateNormalAndDepth()
{
    //std::cout<<"UpdateNormalAndDepth1\n";
    map<boost::interprocess::offset_ptr<KeyFrame> ,tuple<int,int>> observations;
    boost::interprocess::offset_ptr<KeyFrame>  pRefKF;
    cv::Mat Pos;
    {
        std::scoped_lock<mutex> lock1(mMutexFeatures);
        std::scoped_lock<mutex> lock2(mMutexPos);
        if(mbBad)
            return;
        //observations=mObservations;
        observations.insert(mObservations->begin(), mObservations->end());
        pRefKF=mpRefKF;
        Pos = mWorldPos.clone();
    }
    //std::cout<<"UpdateNormalAndDepth2\n";
    if(observations.empty())
        return;

    cv::Mat normal = cv::Mat::zeros(3,1,CV_32F);
    int n=0;
    //std::cout<<"UpdateNormalAndDepth3\n";
    for(map<boost::interprocess::offset_ptr<KeyFrame> ,tuple<int,int>>::iterator mit=observations.begin(), mend=observations.end(); mit!=mend; mit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKF = mit->first;

        tuple<int,int> indexes = mit -> second;
        int leftIndex = get<0>(indexes), rightIndex = get<1>(indexes);

        if(leftIndex != -1){
            cv::Mat Owi = pKF->GetCameraCenter();
            cv::Mat normali = mWorldPos - Owi;
            normal = normal + normali/cv::norm(normali);
            n++;
        }
        if(rightIndex != -1){
            cv::Mat Owi = pKF->GetRightCameraCenter();
            cv::Mat normali = mWorldPos - Owi;
            normal = normal + normali/cv::norm(normali);
            n++;
        }
    }
    //std::cout<<"UpdateNormalAndDepth4\n";
    cv::Mat PC = Pos - pRefKF->GetCameraCenter();
    const float dist = cv::norm(PC);

    tuple<int ,int> indexes = observations[pRefKF];
    int leftIndex = get<0>(indexes), rightIndex = get<1>(indexes);
    int level;
    if(pRefKF -> NLeft == -1){
        level = (*pRefKF->mvKeysUn)[leftIndex].octave;
    }
    else if(leftIndex != -1){
        level = (*pRefKF -> mvKeys)[leftIndex].octave;
    }
    else{
        level = (*pRefKF -> mvKeysRight)[rightIndex - pRefKF -> NLeft].octave;
    }
    //std::cout<<"UpdateNormalAndDepth5\n";
    const float levelScaleFactor =  pRefKF->mvScaleFactors->at(level);//const float levelScaleFactor =  pRefKF->mvScaleFactors[level];
    const int nLevels = pRefKF->mnScaleLevels;

    {
        std::scoped_lock<mutex> lock3(mMutexPos);
        mfMaxDistance = dist*levelScaleFactor;
        mfMinDistance = mfMaxDistance/pRefKF->mvScaleFactors->at(nLevels-1);//mfMinDistance = mfMaxDistance/pRefKF->mvScaleFactors[nLevels-1];
        //mNormalVector = normal/n;
        cv::Mat temp = normal/n;
        temp.copyTo(mNormalVector);
        mNormalVectorx = cv::Matx31f(mNormalVector.at<float>(0), mNormalVector.at<float>(1), mNormalVector.at<float>(2));
    }
    //std::cout<<"UpdateNormalAndDepth6\n";
}

void MapPoint::SetNormalVector(cv::Mat& normal)
{
    std::scoped_lock<mutex> lock3(mMutexPos);
    mNormalVector = normal;
    mNormalVectorx = cv::Matx31f(mNormalVector.at<float>(0), mNormalVector.at<float>(1), mNormalVector.at<float>(2));
}

float MapPoint::GetMinDistanceInvariance()
{
    std::scoped_lock<mutex> lock(mMutexPos);
    return 0.8f*mfMinDistance;
}

float MapPoint::GetMaxDistanceInvariance()
{
    std::scoped_lock<mutex> lock(mMutexPos);
    return 1.2f*mfMaxDistance;
}

int MapPoint::PredictScale(const float &currentDist, boost::interprocess::offset_ptr<KeyFrame>  pKF)
{
    float ratio;
    {
        std::scoped_lock<mutex> lock(mMutexPos);
        ratio = mfMaxDistance/currentDist;
    }

    int nScale = ceil(log(ratio)/pKF->mfLogScaleFactor);
    if(nScale<0)
        nScale = 0;
    else if(nScale>=pKF->mnScaleLevels)
        nScale = pKF->mnScaleLevels-1;

    return nScale;
}

int MapPoint::PredictScale(const float &currentDist, Frame* pF)
{
    float ratio;
    {
        std::scoped_lock<mutex> lock(mMutexPos);
        ratio = mfMaxDistance/currentDist;
    }

    int nScale = ceil(log(ratio)/pF->mfLogScaleFactor);
    if(nScale<0)
        nScale = 0;
    else if(nScale>=pF->mnScaleLevels)
        nScale = pF->mnScaleLevels-1;

    return nScale;
}

boost::interprocess::offset_ptr<Map>  MapPoint::GetMap()
{
    std::scoped_lock<mutex> lock(mMutexMap);
    return mpMap;
}

void MapPoint::UpdateMap(boost::interprocess::offset_ptr<Map>  pMap)
{
    std::scoped_lock<mutex> lock(mMutexMap);
    mpMap = pMap;
}

} //namespace ORB_SLAM
