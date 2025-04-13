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

#include "Atlas.h"
#include "Viewer.h"
#include "System.h"

#include "GeometricCamera.h"
#include "Pinhole.h"
#include "KannalaBrandt8.h"

namespace ORB_SLAM3
{

Atlas::Atlas(){
    //mpCurrentMap = static_cast<boost::interprocess::offset_ptr<Map> >(NULL);
    mpCurrentMap = 0;
    a = 10;
    b = 25;
}

Atlas::Atlas(int initKFid): mnLastInitKFidMap(initKFid), mHasViewer(false)//,segment(boost::interprocess::open_or_create, "MySharedMemory",10737418240), a(10), b(25)
{

    std::cout<<"Atlas initialized:"<<std::endl;
 
    
    //mpCurrentMap = static_cast<boost::interprocess::offset_ptr<Map> >(NULL);
    mpCurrentMap = 0;
    std::pair<int *,std::size_t> ret = ORB_SLAM3::segment.find<int>("magic-num");
    processnum = *ret.first;
    
    CreateNewMap();
}

Atlas::~Atlas()
{
    for(std::set<boost::interprocess::offset_ptr<Map> >::iterator it = mspMaps.begin(), end = mspMaps.end(); it != end;)
    {
        boost::interprocess::offset_ptr<Map>  pMi = *it;

        if(pMi)
        {
            delete pMi.get();
            pMi = static_cast<boost::interprocess::offset_ptr<Map> >(NULL);

            it = mspMaps.erase(it);
        }
        else
            ++it;

    }
}


void Atlas::CreateNewMap()
{
    //new sharing
    //boost::interprocess::managed_shared_memory segment(boost::interprocess::open_or_create, "MySharedMemory",10737418240);
    //std::string name_map = "Map";
    //segment = &segment_mem;
    //Atlas *mpAtlas = (*segment).find<Atlas>("Atlas")().first;
    
    cout<<"In create New Map()"<<endl;
    std::unique_lock<mutex> lock(mMutexAtlas);
    
    
    //cout << "Creation of new map with id: " << Map::nNextId << endl;
    if(mpCurrentMap){
        cout << "Exits current map " << endl;
        if(!mspMaps.empty() && mnLastInitKFidMap < mpCurrentMap->GetMaxKFid())
            mnLastInitKFidMap = mpCurrentMap->GetMaxKFid()+1; //The init KF is the next of current maximum

        mpCurrentMap->SetStoredMap();
        cout << "Saved map with ID: " << mpCurrentMap->GetId() << endl;

        //if(mHasViewer)
        //    mpViewer->AddMapToCreateThumbnail(mpCurrentMap);
    }
    cout << "Creation of new map with last KF id: " << mnLastInitKFidMap << endl;

    //mpCurrentMap = new Map(mnLastInitKFidMap);


    //Aditya: Use shared memory segment to create a new object, give a name with its map ID
    //Open managed shared memory
    
    //std::string string_num_map = to_string(mnLastInitKFidMap);
    //name_map.append(string_num_map);

    //initialize the map now.


    //int flag = std::cin.get();
      /*
    std::pair<int *,std::size_t> ret = ORB_SLAM3::segment.find<int>("magic-num");
    
   
 
    if(ret.first != 0)
    {
         *ret.first++;
    processnum = *ret.first;
    }
else{
        processnum = 1;
}
*/

   
    char mapname[5];

    mapname[0] = 'm';
    mapname[1] = 'a';
    mapname[2] = 'p';
    mapname[4] = '\0';

    std::cout<<"Process num in CreateNewMap "<<processnum<<std::endl;

    sprintf(&mapname[3],"%d",processnum);
    std::cout<<"MapName in create map: "<<mapname<<std::endl;
    mpCurrentMap = ORB_SLAM3::segment.construct<Map>(boost::interprocess::anonymous_instance) (mnLastInitKFidMap);
    //cout<<"Created Map object in shared memory! Address is: "<<mpCurrentMap<<endl;
    //cout<<"Reading a variable there "<<mpCurrentMap->GetMaxKFid()<<endl;

    mpCurrentMap->SetCurrentMap();

    //populate the segment part.. 
    mpCurrentMap->segment = segment;
    //example
    currentMapPtr = mpCurrentMap;

    mspMaps.insert(mpCurrentMap);
    memcpy(currentMapPtr->map_name,mapname,sizeof(char)*5);
}

/* New function written by Aditya to accomodate new maps found in Shared Memory */
void Atlas::AddMap(boost::interprocess::offset_ptr<Map>  pMap)
{
    // We receive a map already in shared memory "pMap"

    cout<<"Inserting the map into Atlas\n";
    //try inserting old map.
    mspMaps.insert(pMap);

}



void Atlas::ChangeMap(boost::interprocess::offset_ptr<Map>  pMap)
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    cout << "Change to map with id: " << pMap->GetId() << endl;
    if(mpCurrentMap){
        mpCurrentMap->SetStoredMap();
    }

    mpCurrentMap = pMap.get();
    mpCurrentMap->SetCurrentMap();
}

unsigned long int Atlas::GetLastInitKFid()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mnLastInitKFidMap;
}

void Atlas::SetViewer(Viewer* pViewer)
{
    mpViewer = pViewer;
    mHasViewer = true;
}

void Atlas::AddKeyFrame(boost::interprocess::offset_ptr<KeyFrame>  pKF)
{
    boost::interprocess::offset_ptr<Map>  pMapKF = pKF->GetMap();
    pMapKF->AddKeyFrame(pKF);

    /*
    // print the sizes of all the matrices
    std::cout<<"----- ==== KEYFRAMES Details ---- ======= \n";
    std::cout<<"mTcwGBA: "<<pKF->mTcwGBA.size<<" "<<pKF->mTcwGBA.total()*pKF->mTcwGBA.elemSize()<<std::endl;
    std::cout<<"mTcwBefGBA: "<<pKF->mTcwBefGBA.size<<" "<<pKF->mTcwBefGBA.total()*pKF->mTcwBefGBA.elemSize()<<std::endl;
    std::cout<<"mVwbGBA: "<<pKF->mVwbGBA.size<<" "<<pKF->mVwbGBA.total()*pKF->mVwbGBA.elemSize()<<std::endl;
    std::cout<<"mVwbBefGBA: "<<pKF->mVwbBefGBA.size<<" "<<pKF->mVwbBefGBA.total()*pKF->mVwbBefGBA.elemSize()<<std::endl;
    
    std::cout<<"----- ==== KEYFRAMES MERGE Details ---- ======= \n";

    std::cout<<"mTcwMerge: "<<pKF->mTcwMerge.size<<" "<<pKF->mTcwMerge.total()*pKF->mTcwMerge.elemSize()<<std::endl;
    std::cout<<"mTcwBefMerge: "<<pKF->mTcwBefMerge.size<<" "<<pKF->mTcwBefMerge.total()*pKF->mTcwBefMerge.elemSize()<<std::endl;
    std::cout<<"mTwcBefMerge: "<<pKF->mTwcBefMerge.size<<" "<<pKF->mTwcBefMerge.total()*pKF->mTwcBefMerge.elemSize()<<std::endl;
    std::cout<<"mVwbMerge: "<<pKF->mVwbMerge.size<<" "<<pKF->mVwbMerge.total()*pKF->mVwbMerge.elemSize()<<std::endl;
    std::cout<<"mVwbBefMerge: "<<pKF->mVwbBefMerge.size<<" "<<pKF->mVwbBefMerge.total()*pKF->mVwbBefMerge.elemSize()<<std::endl;

    std::cout<<"----- ==== KEYFRAMES other matrix Details ---- ======= \n";
    std::cout<<"Tcw: "<<pKF->GetPose().size<<" "<<pKF->GetPose().total()*pKF->GetPose().elemSize()<<std::endl;  
    std::cout<<"Twc: "<<pKF->GetPoseInverse().size<<" "<<pKF->GetPoseInverse().total()*pKF->GetPoseInverse().elemSize()<<std::endl;
    std::cout<<"Ow: "<<pKF->GetCameraCenter().size<<" "<<pKF->GetCameraCenter().total()*pKF->GetCameraCenter().elemSize()<<std::endl;
    std::cout<<"Cw: "<<pKF->GetStereoCenter().size<<" "<<pKF->GetStereoCenter().total()*pKF->GetStereoCenter().elemSize()<<std::endl;
    std::cout<<"Vw: "<<pKF->GetVelocity().size<<" "<<pKF->GetVelocity().total()*pKF->GetVelocity().elemSize()<<std::endl;
    std::cout<<"Owb: "<<pKF->GetImuPosition().size<<" "<<pKF->GetImuPosition().total()*pKF->GetImuPosition().elemSize()<<std::endl;
*/
    std::cout<<"------- ===== Number of Keyframes now             : "<<pMapKF->KeyFramesInMap()<<" ----- ===========\n";
    std::cout<<"------- ===== The Map ID this keyframe Belongs to : "<<pMapKF->GetId()<<" ----- ===========\n";

}

void Atlas::AddMapPoint(boost::interprocess::offset_ptr<MapPoint>  pMP)
{
    boost::interprocess::offset_ptr<Map>  pMapMP = pMP->GetMap();
    pMapMP->AddMapPoint(pMP);
}

vector<GeometricCamera*> Atlas::getCurrentCamera(){
    std::cout<<"Number of Cameras: "<<mvpCameras.size()<<std::endl;
    return mvpCameras;
}

void Atlas::AddCamera(GeometricCamera* pCam)
{
    mvpCameras.push_back(pCam);
}

void Atlas::SetReferenceMapPoints(const std::vector<boost::interprocess::offset_ptr<MapPoint> > &vpMPs)
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    mpCurrentMap->SetReferenceMapPoints(vpMPs);
}

void Atlas::InformNewBigChange()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    mpCurrentMap->InformNewBigChange();
}

int Atlas::GetLastBigChangeIdx()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->GetLastBigChangeIdx();
}

long unsigned int Atlas::MapPointsInMap()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->MapPointsInMap();
}

long unsigned Atlas::KeyFramesInMap()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->KeyFramesInMap();
}

std::vector<boost::interprocess::offset_ptr<KeyFrame> > Atlas::GetAllKeyFrames()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->GetAllKeyFrames();
}

std::vector<boost::interprocess::offset_ptr<MapPoint> > Atlas::GetAllMapPoints()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->GetAllMapPoints();
}

std::vector<boost::interprocess::offset_ptr<MapPoint> > Atlas::GetReferenceMapPoints()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->GetReferenceMapPoints();
}

vector<boost::interprocess::offset_ptr<Map> > Atlas::GetAllMaps()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    struct compFunctor
    {
        inline bool operator()(boost::interprocess::offset_ptr<Map>  elem1 ,boost::interprocess::offset_ptr<Map>  elem2)
        {
            return elem1->GetId() < elem2->GetId();
        }
    };
    vector<boost::interprocess::offset_ptr<Map> > vMaps(mspMaps.begin(),mspMaps.end());
    sort(vMaps.begin(), vMaps.end(), compFunctor());
    return vMaps;
}

int Atlas::CountMaps()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mspMaps.size();
}

void Atlas::clearMap()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    mpCurrentMap->clear();
}

void Atlas::clearAtlas()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    /*for(std::set<boost::interprocess::offset_ptr<Map> >::iterator it=mspMaps.begin(), send=mspMaps.end(); it!=send; it++)
    {
        (*it)->clear();
        delete *it;
    }*/
    mspMaps.clear();
    //mpCurrentMap = static_cast<boost::interprocess::offset_ptr<Map> >(NULL);
    //Aditya
    mpCurrentMap = NULL;
    mnLastInitKFidMap = 0;
}

boost::interprocess::offset_ptr<Map>  Atlas::GetCurrentMap()
//Map* Atlas::GetCurrentMap()
{
    //boost::interprocess::managed_shared_memory segment(boost::interprocess::open_or_create, "MySharedMemory",10737418240);
    //Atlas *atl = segment->find_or_construct<Atlas>("Atlas")();

    std::unique_lock<mutex> lock(mMutexAtlas);
    
    if(!mpCurrentMap)
        CreateNewMap();

    //previous code to give map. just gives default map
    /*
    if(!atl->mpCurrentMap)
        CreateNewMap();
    */

/*

    cout<<"Checked if new map is required. GetCurrentMap()"<<endl;
    char mapname[5];

    mapname[0] = 'm';
    mapname[1] = 'a';
    mapname[2] = 'p';
    mapname[4] = '\0';

    sprintf(&mapname[3],"%d",processnum);

    
    std::cout<<"MapName in getcurrent map: "<<mapname<<std::endl;
    std::pair<Map *,std::size_t> ret = segment->find<Map>(mapname);
    if (ret.first == 0)
    {
        std::cout<<"Cannot find Map. Making a map now\n";
        //segment->construct<Map>("Map1")();
        CreateNewMap();
    }
    else{
        mpCurrentMap = ret.first;
        std::cout<<"Found an existing map"<<std::endl;
    }
*/
   
    //run functions.
    //cout<<"GetId: "<<mpCurrentMap->GetId()<<endl;
    //cout<<"Address of mpCurrentMap "<<mpCurrentMap<<endl;

    //let's see if we can access mpCurrentMap
    //cout<<"mpCurrentMap->nNextId: "<<mpCurrentMap->nNextId<<endl;
    //check boolean
    /*
    if(mpCurrentMap->mbBad)
        cout<<"mbBad is true"<<endl;
    else
        cout<<"mbBad is false"<<endl;

    */
    while(mpCurrentMap->IsBad())
        usleep(3000);


    return mpCurrentMap;
}

void Atlas::SetMapBad(boost::interprocess::offset_ptr<Map>  pMap)
{
    mspMaps.erase(pMap);
    pMap->SetBad();

    mspBadMaps.insert(pMap);
}

void Atlas::RemoveBadMaps()
{
    /*for(boost::interprocess::offset_ptr<Map>  pMap : mspBadMaps)
    {
        delete pMap;
        pMap = static_cast<boost::interprocess::offset_ptr<Map> >(NULL);
    }*/
    mspBadMaps.clear();
}

bool Atlas::isInertial()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->IsInertial();
}

void Atlas::SetInertialSensor()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    mpCurrentMap->SetInertialSensor();
}

void Atlas::SetImuInitialized()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    mpCurrentMap->SetImuInitialized();
}

bool Atlas::isImuInitialized()
{
    //boost::interprocess::managed_shared_memory segment(boost::interprocess::open_or_create, "MySharedMemory",10737418240);
    //Atlas *atl = segment->find_or_construct<Atlas>("Atlas")();

    //cout<<"Address of mpCurrentMap "<<mpCurrentMap<<endl;
    std::unique_lock<mutex> lock(mMutexAtlas);
    return mpCurrentMap->isImuInitialized();
}

void Atlas::SetKeyFrameDababase(KeyFrameDatabase* pKFDB)
{
    mpKeyFrameDB = pKFDB;
}


int Atlas::sum_of_two(){
    cout<<"Add a "<<a<<" and b "<<b<<" sum "<<(a+b)<<endl;
    return (a+b);
}

KeyFrameDatabase* Atlas::GetKeyFrameDatabase()
{
    return mpKeyFrameDB;
}

void Atlas::SetORBVocabulary(ORBVocabulary* pORBVoc)
{
    mpORBVocabulary = pORBVoc;
}

ORBVocabulary* Atlas::GetORBVocabulary()
{
    return mpORBVocabulary;
}

long unsigned int Atlas::GetNumLivedKF()
{
    std::unique_lock<mutex> lock(mMutexAtlas);
    long unsigned int num = 0;
    for(boost::interprocess::offset_ptr<Map>  mMAPi : mspMaps)
    {
        num += mMAPi->GetAllKeyFrames().size();
    }

    return num;
}

long unsigned int Atlas::GetNumLivedMP() {
    std::unique_lock<mutex> lock(mMutexAtlas);
    long unsigned int num = 0;
    for (boost::interprocess::offset_ptr<Map> mMAPi : mspMaps) {
        num += mMAPi->GetAllMapPoints().size();
    }

    return num;
}

} //namespace ORB_SLAM3
