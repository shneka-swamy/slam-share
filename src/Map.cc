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


#include "Map.h"
#include "System.h"

#include<mutex>

namespace ORB_SLAM3
{

// Shared memory declarations
//Define an STL compatible allocator of ints that allocates from the managed_shared_memory.
//This allocator will allow placing containers in the segment
//typedef boost::interprocess::allocator<Map, boost::interprocess::managed_shared_memory::segment_manager>  ShmemAllocator;


long unsigned int Map::nNextId=0;

Map::Map():mnMaxKFid(0),mnBigChangeIdx(0), mbImuInitialized(false), mnMapChange(0), mpFirstRegionKF(static_cast<boost::interprocess::offset_ptr<KeyFrame> >(NULL)),
mbFail(false), mIsInUse(false), mHasTumbnail(false), mbBad(false), mnMapChangeNotified(0), mbIsInertial(false), mbIMU_BA1(false), mbIMU_BA2(false)
{
    //shared memory initialization for the map
    /*
    //off for now
    boost::interprocess::managed_shared_memory shm_temp(boost::interprocess::open_only, "MySharedMemory");
    shm = &shm_temp;
    */
    const ShmemAllocator alloc_inst(ORB_SLAM3::segment.get_segment_manager());
    mvpKeyFrameOrigins = ORB_SLAM3::segment.construct<MyVector>(boost::interprocess::anonymous_instance)(alloc_inst);
    keyframeorigins_offsetptr = mvpKeyFrameOrigins;

    const ShmemAllocator_longint alloc_inst2(ORB_SLAM3::segment.get_segment_manager());
    mvBackupKeyFrameOriginsId = ORB_SLAM3::segment.construct<MyVector_longint>(boost::interprocess::anonymous_instance)(alloc_inst2);

    const ShmemAllocator_keyframe_set alloc_set_key(ORB_SLAM3::segment.get_segment_manager());
    mspKeyFrames = ORB_SLAM3::segment.construct<Myset_keyframe>(boost::interprocess::anonymous_instance)(alloc_set_key);

    const ShmemAllocator_mappoint_set alloc_set_mappoint(ORB_SLAM3::segment.get_segment_manager());
    mspMapPoints = ORB_SLAM3::segment.construct<Myset_mappoint>(boost::interprocess::anonymous_instance)(alloc_set_mappoint);

    const ShmemAllocator_mappoint alloc_inst_mappoint(ORB_SLAM3::segment.get_segment_manager());
    mvpReferenceMapPoints = ORB_SLAM3::segment.construct<MyVector_mappoint>(boost::interprocess::anonymous_instance)(alloc_inst_mappoint);

    mnId=nNextId++;
    mThumbnail = static_cast<GLubyte*>(NULL);
}

Map::Map(int initKFid):mnInitKFid(initKFid), mnMaxKFid(initKFid),mnLastLoopKFid(initKFid), mnBigChangeIdx(0), mIsInUse(false),
                       mHasTumbnail(false), mbBad(false), mbImuInitialized(false), mpFirstRegionKF(static_cast<boost::interprocess::offset_ptr<KeyFrame> >(NULL)),
                       mnMapChange(0), mbFail(false), mnMapChangeNotified(0), mbIsInertial(false), mbIMU_BA1(false), mbIMU_BA2(false)
{
    
    //off for now
    //boost::interprocess::managed_shared_memory shm_temp(boost::interprocess::open_only, "MySharedMemory");
    //shm = &shm_temp;
    

    //std::cout<<"Map init called?"<<std::endl;
    //std::cout<<"Updating a and B"<<std::endl;
    //a = 10;
    //b = 25;

    const ShmemAllocator alloc_inst(ORB_SLAM3::segment.get_segment_manager());
    mvpKeyFrameOrigins = ORB_SLAM3::segment.construct<MyVector>(boost::interprocess::anonymous_instance)(alloc_inst);
    keyframeorigins_offsetptr = mvpKeyFrameOrigins;


    const ShmemAllocator_longint alloc_inst2(ORB_SLAM3::segment.get_segment_manager());
    mvBackupKeyFrameOriginsId = ORB_SLAM3::segment.construct<MyVector_longint>(boost::interprocess::anonymous_instance)(alloc_inst2);

    //also initialize the mutex.
    mMutexMapPtr = &mMutexMap;

    mnId=nNextId++;
    mThumbnail = static_cast<GLubyte*>(NULL);

    //for the sets
    const ShmemAllocator_keyframe_set alloc_set_key(ORB_SLAM3::segment.get_segment_manager());
    mspKeyFrames = ORB_SLAM3::segment.construct<Myset_keyframe>(boost::interprocess::anonymous_instance)(alloc_set_key);

    //for mappoint reference
     const ShmemAllocator_mappoint alloc_inst_mappoint(ORB_SLAM3::segment.get_segment_manager());
    mvpReferenceMapPoints = ORB_SLAM3::segment.construct<MyVector_mappoint>(boost::interprocess::anonymous_instance)(alloc_inst_mappoint);

    const ShmemAllocator_mappoint_set alloc_set_mappoint(ORB_SLAM3::segment.get_segment_manager());
    mspMapPoints = ORB_SLAM3::segment.construct<Myset_mappoint>(boost::interprocess::anonymous_instance)(alloc_set_mappoint);


}

Map::~Map()
{
    //TODO: erase all points from memory
    //mspMapPoints.clear();
    mspMapPoints->clear();

    //TODO: erase all keyframes from memory
    //old
    //mspKeyFrames.clear();
    //new
    mspKeyFrames->clear();

    if(mThumbnail)
        delete mThumbnail;
    mThumbnail = static_cast<GLubyte*>(NULL);

    //old
    //mvpReferenceMapPoints.clear();
    //new
    mvpReferenceMapPoints->clear();
    //old
    //mvpKeyFrameOrigins.clear();
    //new
    mvpKeyFrameOrigins->clear();
}

void Map::AddKeyFrame(boost::interprocess::offset_ptr<KeyFrame> pKF)
{
    std::unique_lock<mutex> lock(mMutexMap);
    //if(mspKeyFrames.empty()){
    if(mspKeyFrames->empty()){
        cout << "First KF:" << pKF->mnId << "; Map init KF:" << mnInitKFid << endl;
        mnInitKFid = pKF->mnId;
        mpKFinitial = pKF;
        mpKFlowerID = pKF;
    }
    mspKeyFrames->insert(pKF);//mspKeyFrames.insert(pKF);
    if(pKF->mnId>mnMaxKFid)
    {
        mnMaxKFid=pKF->mnId;
    }
    if(pKF->mnId<mpKFlowerID->mnId)
    {
        mpKFlowerID = pKF;
    }
}

void Map::AddMapPoint(boost::interprocess::offset_ptr<MapPoint> pMP)
{
    std::unique_lock<mutex> lock(mMutexMap);
    //mspMapPoints.insert(pMP);
    mspMapPoints->insert(pMP);
}

void Map::SetImuInitialized()
{
    std::unique_lock<mutex> lock(mMutexMap);
    mbImuInitialized = true;
}

bool Map::isImuInitialized()
{
    //cout<<"isImuInitialized called"<<endl;
    //Atlas *atl;
    std::unique_lock<mutex> lock(this->mMutexMap);
    return mbImuInitialized;
}

void Map::EraseMapPoint(boost::interprocess::offset_ptr<MapPoint> pMP)
{
    std::unique_lock<mutex> lock(mMutexMap);
    mspMapPoints->erase(pMP);//mspMapPoints.erase(pMP);

    // TODO: This only erase the pointer.
    // Delete the MapPoint
}

void Map::EraseKeyFrame(boost::interprocess::offset_ptr<KeyFrame> pKF)
{
    std::unique_lock<mutex> lock(mMutexMap);
    mspKeyFrames->erase(pKF);//mspKeyFrames.erase(pKF);
    if(mspKeyFrames->size()>0)//if(mspKeyFrames.size()>0)
    {
        if(pKF->mnId == mpKFlowerID->mnId)
        {
            //vector<boost::interprocess::offset_ptr<KeyFrame> > vpKFs = vector<boost::interprocess::offset_ptr<KeyFrame> >(mspKeyFrames.begin(),mspKeyFrames.end());
            vector<boost::interprocess::offset_ptr<KeyFrame> > vpKFs = vector<boost::interprocess::offset_ptr<KeyFrame> >(mspKeyFrames->begin(),mspKeyFrames->end());
            //sort(vpKFs.begin(),vpKFs.end(),KeyFrame::lId);
            sort(vpKFs.begin(),vpKFs.end(),KeyFrame::lId);
            mpKFlowerID = vpKFs[0];
        }
    }
    else
    {
        mpKFlowerID = 0;
    }

    // TODO: This only erase the pointer.
    // Delete the MapPoint
}

void Map::SetReferenceMapPoints(const vector<boost::interprocess::offset_ptr<MapPoint> > &vpMPs)
{
    std::unique_lock<mutex> lock(mMutexMap);
    mvpReferenceMapPoints->clear();
    mvpReferenceMapPoints->assign(vpMPs.begin(),vpMPs.end());
    //mvpReferenceMapPoints = vpMPs;
}

void Map::InformNewBigChange()
{
    std::unique_lock<mutex> lock(mMutexMap);
    mnBigChangeIdx++;
}

int Map::GetLastBigChangeIdx()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mnBigChangeIdx;
}

vector<boost::interprocess::offset_ptr<KeyFrame> > Map::GetAllKeyFrames()
{
    std::unique_lock<mutex> lock(mMutexMap);
    //return vector<boost::interprocess::offset_ptr<KeyFrame> >(mspKeyFrames.begin(),mspKeyFrames.end());
    return vector<boost::interprocess::offset_ptr<KeyFrame> >(mspKeyFrames->begin(),mspKeyFrames->end());
}

vector<boost::interprocess::offset_ptr<MapPoint> > Map::GetAllMapPoints()
{
    std::unique_lock<mutex> lock(mMutexMap);
    std::vector<boost::interprocess::offset_ptr<MapPoint> > returnable;
    //mspMapPoints_support.clear();
    for (auto f: *mspMapPoints)
    {
        //mspMapPoints_support.insert(f);
        returnable.push_back(f);
    }
    //old-code
    //return vector<boost::interprocess::offset_ptr<MapPoint> >(mspMapPoints.begin(),mspMapPoints.end());
    //return vector<boost::interprocess::offset_ptr<MapPoint> >(mspMapPoints_support.begin(),mspMapPoints_support.end());
    return returnable;
}

long unsigned int Map::MapPointsInMap()
{
    std::unique_lock<mutex> lock(mMutexMap);
    //return mspMapPoints.size();
    return mspMapPoints->size();
}

long unsigned int Map::KeyFramesInMap()
{
    //std::unique_lock<mutex> lock(mMutexMap);
    //return mspKeyFrames.size();
    return mspKeyFrames->size();
}

vector<boost::interprocess::offset_ptr<MapPoint> > Map::GetReferenceMapPoints()
{
    std::unique_lock<mutex> lock(mMutexMap);
    //mvpReferenceMapPoints_support.clear();
    std::vector<boost::interprocess::offset_ptr<MapPoint> > returnable;
    for(auto f: *mvpReferenceMapPoints)
    {
        //mvpReferenceMapPoints_support.push_back(f);
        returnable.push_back(f);
    }
    //return mvpReferenceMapPoints_support; //changed here
    return returnable;
}

long unsigned int Map::GetId()
{
    return mnId;
}
long unsigned int Map::GetInitKFid()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mnInitKFid;
}

void Map::SetInitKFid(long unsigned int initKFif)
{
    std::unique_lock<mutex> lock(mMutexMap);
    mnInitKFid = initKFif;
}

long unsigned int Map::GetMaxKFid()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mnMaxKFid;
}

boost::interprocess::offset_ptr<KeyFrame> Map::GetOriginKF()
{
    return mpKFinitial;
}

void Map::SetCurrentMap()
{
    mIsInUse = true;
}

void Map::SetStoredMap()
{
    mIsInUse = false;
}

void Map::clear()
{
//    for(set<boost::interprocess::offset_ptr<MapPoint> >::iterator sit=mspMapPoints.begin(), send=mspMapPoints.end(); sit!=send; sit++)
//        delete *sit;

    //for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames.begin(), send=mspKeyFrames.end(); sit!=send; sit++)
    mspKeyFrames_support.clear();
    for(auto f : *mspKeyFrames){
       mspKeyFrames_support.insert(f);
   }

    //for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames.begin(); sit!=mspKeyFrames.end(); sit++)
    for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames_support.begin(); sit!=mspKeyFrames_support.end(); sit++)
    //for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames->begin(), send=mspKeyFrames->end(); sit!=send; sit++)
    {
        boost::interprocess::offset_ptr<KeyFrame> pKF = *sit;
        pKF->UpdateMap(static_cast<boost::interprocess::offset_ptr<Map> >(NULL));
//        delete *sit;
    }

    mspMapPoints->clear();//mspMapPoints.clear();
    mspKeyFrames->clear();//mspKeyFrames.clear();
    mnMaxKFid = mnInitKFid;
    mnLastLoopKFid = 0;
    mbImuInitialized = false;
    
    //old-code
    //mvpReferenceMapPoints.clear();
    //new-code
    mvpReferenceMapPoints->clear();
    //old
    //mvpKeyFrameOrigins.clear();
    //new
    mvpKeyFrameOrigins->clear();
    mbIMU_BA1 = false;
    mbIMU_BA2 = false;
}

bool Map::IsInUse()
{
    return mIsInUse;
}

void Map::SetBad()
{
    mbBad = true;
}

bool Map::IsBad()
{
    /*
    std::cout<<"Called to see if map is bad?"<<std::endl;
    std::cout<<"print a integer: "<<mnBigChangeIdx<<endl;
    if(this->mbBad)
        std::cout<<"mbBad is true"<<std::endl;
    else
        std::cout<<"mbBad is not true"<<std::endl;
    */
    return mbBad;
}

void Map::RotateMap(const cv::Mat &R)
{
    std::unique_lock<mutex> lock(mMutexMap);

    cv::Mat Txw = cv::Mat::eye(4,4,CV_32F);
    R.copyTo(Txw.rowRange(0,3).colRange(0,3));

    //old
    //boost::interprocess::offset_ptr<KeyFrame> pKFini = mvpKeyFrameOrigins[0];
    //new
    boost::interprocess::offset_ptr<KeyFrame> pKFini = mvpKeyFrameOrigins->at(0);
    
    cv::Mat Twc_0 = pKFini->GetPoseInverse();
    cv::Mat Txc_0 = Txw*Twc_0;
    cv::Mat Txb_0 = Txc_0*pKFini->mImuCalib.Tcb;
    cv::Mat Tyx = cv::Mat::eye(4,4,CV_32F);
    Tyx.rowRange(0,3).col(3) = -Txb_0.rowRange(0,3).col(3);
    cv::Mat Tyw = Tyx*Txw;
    cv::Mat Ryw = Tyw.rowRange(0,3).colRange(0,3);
    cv::Mat tyw = Tyw.rowRange(0,3).col(3);

    mspKeyFrames_support.clear();
    for(auto f : *mspKeyFrames){
       mspKeyFrames_support.insert(f);
   }

    //for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames.begin(); sit!=mspKeyFrames.end(); sit++)
    for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames_support.begin(); sit!=mspKeyFrames_support.end(); sit++)
    {
        boost::interprocess::offset_ptr<KeyFrame> pKF = *sit;
        cv::Mat Twc = pKF->GetPoseInverse();
        cv::Mat Tyc = Tyw*Twc;
        cv::Mat Tcy = cv::Mat::eye(4,4,CV_32F);
        Tcy.rowRange(0,3).colRange(0,3) = Tyc.rowRange(0,3).colRange(0,3).t();
        Tcy.rowRange(0,3).col(3) = -Tcy.rowRange(0,3).colRange(0,3)*Tyc.rowRange(0,3).col(3);
        pKF->SetPose(Tcy);
        cv::Mat Vw = pKF->GetVelocity();
        pKF->SetVelocity(Ryw*Vw);
    }
    mspMapPoints_support.clear();
    for (auto f: *mspMapPoints)
    {
        mspMapPoints_support.insert(f);
    }
    //old-code
    //for(set<boost::interprocess::offset_ptr<MapPoint> >::iterator sit=mspMapPoints.begin(); sit!=mspMapPoints.end(); sit++)
    for(set<boost::interprocess::offset_ptr<MapPoint> >::iterator sit=mspMapPoints_support.begin(); sit!=mspMapPoints_support.end(); sit++)
    {
        boost::interprocess::offset_ptr<MapPoint> pMP = *sit;
        pMP->SetWorldPos(Ryw*pMP->GetWorldPos()+tyw);
        pMP->UpdateNormalAndDepth();
    }
}

void Map::ApplyScaledRotation(const cv::Mat &R, const float s, const bool bScaledVel, const cv::Mat t)
{
    std::unique_lock<mutex> lock(mMutexMap);

    // Body position (IMU) of first keyframe is fixed to (0,0,0)
    cv::Mat Txw = cv::Mat::eye(4,4,CV_32F);
    R.copyTo(Txw.rowRange(0,3).colRange(0,3));

    cv::Mat Tyx = cv::Mat::eye(4,4,CV_32F);

    cv::Mat Tyw = Tyx*Txw;
    Tyw.rowRange(0,3).col(3) = Tyw.rowRange(0,3).col(3)+t;
    cv::Mat Ryw = Tyw.rowRange(0,3).colRange(0,3);
    cv::Mat tyw = Tyw.rowRange(0,3).col(3);

    mspKeyFrames_support.clear();
    for(auto f : *mspKeyFrames){
       mspKeyFrames_support.insert(f);
    }
    //mspKeyFrames_support.assign(mspKeyFrames->begin(), mspKeyFrames->end());

    //for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames.begin(); sit!=mspKeyFrames.end(); sit++)
    for(set<boost::interprocess::offset_ptr<KeyFrame> >::iterator sit=mspKeyFrames_support.begin(); sit!=mspKeyFrames_support.end(); sit++)
    {
        boost::interprocess::offset_ptr<KeyFrame> pKF = *sit;
        cv::Mat Twc = pKF->GetPoseInverse();
        Twc.rowRange(0,3).col(3)*=s;
        cv::Mat Tyc = Tyw*Twc;
        cv::Mat Tcy = cv::Mat::eye(4,4,CV_32F);
        Tcy.rowRange(0,3).colRange(0,3) = Tyc.rowRange(0,3).colRange(0,3).t();
        Tcy.rowRange(0,3).col(3) = -Tcy.rowRange(0,3).colRange(0,3)*Tyc.rowRange(0,3).col(3);
        pKF->SetPose(Tcy);
        cv::Mat Vw = pKF->GetVelocity();
        if(!bScaledVel)
            pKF->SetVelocity(Ryw*Vw);
        else
            pKF->SetVelocity(Ryw*Vw*s);

    }
    mspMapPoints_support.clear();
    for (auto f: *mspMapPoints)
    {
        mspMapPoints_support.insert(f);
    }
    //for(set<boost::interprocess::offset_ptr<MapPoint> >::iterator sit=mspMapPoints.begin(); sit!=mspMapPoints.end(); sit++)
    for(set<boost::interprocess::offset_ptr<MapPoint> >::iterator sit=mspMapPoints_support.begin(); sit!=mspMapPoints_support.end(); sit++)
    {
        boost::interprocess::offset_ptr<MapPoint> pMP = *sit;
        pMP->SetWorldPos(s*Ryw*pMP->GetWorldPos()+tyw);
        pMP->UpdateNormalAndDepth();
    }
    mnMapChange++;
}

/*
void * Map::operator new(size_t _size){
    //allocate the given size form the shared memory
    //we do not need the size as we will be making object

}
*/

void Map::SetInertialSensor()
{
    std::unique_lock<mutex> lock(mMutexMap);
    mbIsInertial = true;
}

bool Map::IsInertial()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mbIsInertial;
}

void Map::SetIniertialBA1()
{
    std::unique_lock<mutex> lock(mMutexMap);
    mbIMU_BA1 = true;
}

void Map::SetIniertialBA2()
{
    std::unique_lock<mutex> lock(mMutexMap);
    mbIMU_BA2 = true;
}

bool Map::GetIniertialBA1()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mbIMU_BA1;
}

bool Map::GetIniertialBA2()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mbIMU_BA2;
}

void Map::PrintEssentialGraph()
{
    //Print the essential graph
    //old
    //vector<boost::interprocess::offset_ptr<KeyFrame> > vpOriginKFs = mvpKeyFrameOrigins;
    //new
    vector<boost::interprocess::offset_ptr<KeyFrame> > vpOriginKFs;
    for(boost::interprocess::offset_ptr<KeyFrame> i:*mvpKeyFrameOrigins)
    {
        vpOriginKFs.push_back(i);
    }
    

    int count=0;
    cout << "Number of origin KFs: " << vpOriginKFs.size() << endl;
    //boost::interprocess::offset_ptr<KeyFrame> pFirstKF;
    KeyFrame *pFirstKF;
    for(boost::interprocess::offset_ptr<KeyFrame> pKFi : vpOriginKFs)
    {
        if(!pFirstKF)
            pFirstKF = pKFi.get();
        else if(!pKFi->GetParent())
            pFirstKF = pKFi.get();
    }
    if(pFirstKF->GetParent())
    {
        cout << "First KF in the essential graph has a parent, which is not possible" << endl;
    }

    cout << "KF: " << pFirstKF->mnId << endl;
    set<boost::interprocess::offset_ptr<KeyFrame> > spChilds = pFirstKF->GetChilds();
    vector<boost::interprocess::offset_ptr<KeyFrame> > vpChilds;
    vector<string> vstrHeader;
    for(boost::interprocess::offset_ptr<KeyFrame> pKFi : spChilds){
        vstrHeader.push_back("--");
        vpChilds.push_back(pKFi);
    }
    //for(int i=0; i<vpChilds.size() && count <= (mspKeyFrames.size()+10); ++i)
    for(int i=0; i<vpChilds.size() && count <= (mspKeyFrames->size()+10); ++i)
    {
        count++;
        string strHeader = vstrHeader[i];
        boost::interprocess::offset_ptr<KeyFrame> pKFi = vpChilds[i];

        cout << strHeader << "KF: " << pKFi->mnId << endl;

        set<boost::interprocess::offset_ptr<KeyFrame> > spKFiChilds = pKFi->GetChilds();
        for(boost::interprocess::offset_ptr<KeyFrame> pKFj : spKFiChilds)
        {
            vpChilds.push_back(pKFj);
            vstrHeader.push_back(strHeader+"--");
        }
    }
    //if (count == (mspKeyFrames.size()+10))
    if (count == (mspKeyFrames->size()+10))
        cout << "CYCLE!!"    << endl;

    cout << "------------------" << endl << "End of the essential graph" << endl;
}

bool Map::CheckEssentialGraph(){
    //old
    //vector<boost::interprocess::offset_ptr<KeyFrame> > vpOriginKFs = mvpKeyFrameOrigins;
    //new
    vector<boost::interprocess::offset_ptr<KeyFrame> > vpOriginKFs;
    for(boost::interprocess::offset_ptr<KeyFrame> i:*mvpKeyFrameOrigins)
    {
        vpOriginKFs.push_back(i);
    }

    int count=0;
    cout << "Number of origin KFs: " << vpOriginKFs.size() << endl;
    boost::interprocess::offset_ptr<KeyFrame> pFirstKF;
    for(boost::interprocess::offset_ptr<KeyFrame> pKFi : vpOriginKFs)
    {
        if(!pFirstKF)
            pFirstKF = pKFi;
        else if(!pKFi->GetParent())
            pFirstKF = pKFi;
    }
    cout << "Checking if the first KF has parent" << endl;
    if(pFirstKF->GetParent())
    {
        cout << "First KF in the essential graph has a parent, which is not possible" << endl;
        cout<<"ID of the parent of "<<pFirstKF->mnId<<" is "<<pFirstKF->GetParent()->mnId;
    }

    set<boost::interprocess::offset_ptr<KeyFrame> > spChilds = pFirstKF->GetChilds();
    vector<boost::interprocess::offset_ptr<KeyFrame> > vpChilds;
    vpChilds.reserve(mspKeyFrames->size());//vpChilds.reserve(mspKeyFrames.size());
    for(boost::interprocess::offset_ptr<KeyFrame> pKFi : spChilds)
        vpChilds.push_back(pKFi);

    //for(int i=0; i<vpChilds.size() && count <= (mspKeyFrames.size()+10); ++i)
    for(int i=0; i<vpChilds.size() && count <= (mspKeyFrames->size()+10); ++i)
    {
        count++;
        boost::interprocess::offset_ptr<KeyFrame> pKFi = vpChilds[i];
        set<boost::interprocess::offset_ptr<KeyFrame> > spKFiChilds = pKFi->GetChilds();
        for(boost::interprocess::offset_ptr<KeyFrame> pKFj : spKFiChilds)
            vpChilds.push_back(pKFj);
    }

    //cout << "count/tot" << count << "/" << mspKeyFrames.size() << endl;
    cout << "count/tot" << count << "/" << mspKeyFrames->size() << endl;
    //if (count != (mspKeyFrames.size()-1))
    if (count != (mspKeyFrames->size()-1))
        return false;
    else
        return true;
}

void Map::ChangeId(long unsigned int nId)
{
    mnId = nId;
}

unsigned int Map::GetLowerKFID()
{
    std::unique_lock<mutex> lock(mMutexMap);
    if (mpKFlowerID) {
        return mpKFlowerID->mnId;
    }
    return 0;
}

int Map::GetMapChangeIndex()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mnMapChange;
}

void Map::IncreaseChangeIndex()
{
    std::unique_lock<mutex> lock(mMutexMap);
    mnMapChange++;
}

int Map::GetLastMapChange()
{
    std::unique_lock<mutex> lock(mMutexMap);
    return mnMapChangeNotified;
}

void Map::SetLastMapChange(int currentChangeId)
{
    std::unique_lock<mutex> lock(mMutexMap);
    mnMapChangeNotified = currentChangeId;
}


} //namespace ORB_SLAM3
