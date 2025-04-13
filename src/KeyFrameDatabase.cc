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


#include "KeyFrameDatabase.h"

#include "KeyFrame.h"
#include "Thirdparty/DBoW2/DBoW2/BowVector.h"

#include<mutex>
#include<cfloat>

using namespace std;

namespace ORB_SLAM3
{

KeyFrameDatabase::KeyFrameDatabase (const ORBVocabulary &voc):
    mpVoc(&voc)
{
    mvInvertedFile.resize(voc.size());
}


void KeyFrameDatabase::add(boost::interprocess::offset_ptr<KeyFrame> pKF)
{
    /*
    std::cout<<"KeyFrameDatabase::add1: Pk-id "<<pKF->mnId<<std::endl;
    DBoW2::BowVector newBow = DBoW2::BowVector();// = mBowVec;
    DBoW2::FeatureVector newFeat = DBoW2::FeatureVector();
    cv::Mat descriptor_mat;
    pKF->mDescriptors.copyTo(descriptor_mat);
    std::vector<cv::Mat> vCurrentDesc;

    vCurrentDesc.reserve(descriptor_mat.rows);
    for (int j=0;j<descriptor_mat.rows;j++)
        vCurrentDesc.push_back(descriptor_mat.row(j));
    
             
        //pKF->mBowVec = mBowVec;
        //pKF->mFeatVec = mFeatVec;
        std::cout<<"Fails here? descriptor size: "<<descriptor_mat<<std::endl;
        //vCurrentDesc = Converter::toDescriptorVector(descriptor_mat);
        // Feature vector associate features with nodes in the 4th level (from leaves up)
        // We assume the vocabulary tree has 6 levels, change the 4 otherwise
        //std::cout<<"Or fails here. vCurrentDesc size: "<<vCurrentDesc.size()<<std::endl;
        mpVoc->transform(vCurrentDesc,newBow,newFeat,4);

    
    std::cout<<"KeyFrameDatabase::add entry"<<newBow.size()<<std::endl;
    //std::unique_lock<mutex> lock(mMutex);
    //std::cout<<"KeyFrameDatabase::after lock. mBowVec Size: "<<pKF->mBowVec.size()<<" mvInvertedFile size: "<<mvInvertedFile.size()<<std::endl;
    */
    int counter = 0;
    // forgive me compiler I hope auto iterator will work.
    //for(DBoW2::BowVector::const_iterator vit= pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit!=vend; vit++){
    
    for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++){

    //for(DBoW2::BowVector::const_iterator vit= newBow.begin(), vend=newBow.end(); vit!=vend; vit++){
        //std::cout<<"Crash?"<<std::endl;
        //std::cout<<"BowVec output: "<<vit->first<<std::endl;
        counter++;
        mvInvertedFile[vit->first].push_back(pKF);
        //if(counter>=pKF->mBowVec.size())
        if(counter>=pKF->mBowVec->size())
            break;
    }
    //std::cout<<"KeyFrameDatabase::add2\n";
    //std::cout<<"Finished pushing the keyframe into Inverted File\n";
}

void KeyFrameDatabase::erase(boost::interprocess::offset_ptr<KeyFrame>  pKF)
{
    
    //temp-temp-temp-- don't erase anything
    //return;

    //remove
    //std::unique_lock<mutex> lock(mMutex);
    std::cout<<"KeyFrameDatabase erase 1\n";

    int counter = 0;

    //pKF->FixBow(pKF,nullptr);
    // Erase elements in the Inverse File for the entry
    //for(DBoW2::BowVector::const_iterator vit=pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit!=vend; vit++)
    for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++)
    {
        std::cout<<"KeyFrameDatabase erase 1.1\n";
        std::cout<<"KeyFrameDatabase erase 1.2 keyframe: "<<pKF->mnId<<std::endl;
        std::cout<<"vit->first: "<<vit->first<<std::endl;
        std::cout<<"mvInvertedFile size: "<<mvInvertedFile.size()<<std::endl;
        // List of keyframes that share the word
        list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs =   mvInvertedFile[vit->first];
        std::cout<<"KeyFrameDatabase erase 2\n";
        counter++;

        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
        {
            std::cout<<"KeyFrameDatabase erase 3\n";
            if(*lit){
                if(pKF==*lit)
                    {
                    lKFs.erase(lit);
                    break;
                    }   
            }
        }
        //if(counter>=pKF->mBowVec.size())
        if(counter>=pKF->mBowVec->size())
            break;
    }
}

void KeyFrameDatabase::clear()
{
    mvInvertedFile.clear();
    mvInvertedFile.resize(mpVoc->size());
}



void KeyFrameDatabase::clearMap(boost::interprocess::offset_ptr<Map>  pMap)
{
    std::unique_lock<mutex> lock(mMutex);

    // Erase elements in the Inverse File for the entry
    for(std::vector<list<boost::interprocess::offset_ptr<KeyFrame> > >::iterator vit=mvInvertedFile.begin(), vend=mvInvertedFile.end(); vit!=vend; vit++)
    {
        // List of keyframes that share the word
        list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs =  *vit;

        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend;)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;
            if(pMap == pKFi->GetMap())
            {
                lit = lKFs.erase(lit);
                // Dont delete the KF because the class Map clean all the KF when it is destroyed
            }
            else
            {
                ++lit;
            }
        }
    }
}

vector<boost::interprocess::offset_ptr<KeyFrame> > KeyFrameDatabase::DetectLoopCandidates(boost::interprocess::offset_ptr<KeyFrame>  pKF, float minScore)
{
    set<boost::interprocess::offset_ptr<KeyFrame> > spConnectedKeyFrames = pKF->GetConnectedKeyFrames();
    list<boost::interprocess::offset_ptr<KeyFrame> > lKFsSharingWords;

    // Search all keyframes that share a word with current keyframes
    // Discard keyframes connected to the query keyframe
    {
        std::unique_lock<mutex> lock(mMutex);

        //for(DBoW2::BowVector::const_iterator vit=pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit != vend; vit++)
        for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++)
        {
            list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs =   mvInvertedFile[vit->first];

            for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi=*lit;
                if(pKFi->GetMap()==pKF->GetMap()) // For consider a loop candidate it a candidate it must be in the same map
                {
                    if(pKFi->mnLoopQuery!=pKF->mnId)
                    {
                        pKFi->mnLoopWords=0;
                        if(!spConnectedKeyFrames.count(pKFi))
                        {
                            pKFi->mnLoopQuery=pKF->mnId;
                            lKFsSharingWords.push_back(pKFi);
                        }
                    }
                    pKFi->mnLoopWords++;
                }


            }
        }
    }

    if(lKFsSharingWords.empty())
        return vector<boost::interprocess::offset_ptr<KeyFrame> >();

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lScoreAndMatch;

    // Only compare against those keyframes that share enough words
    int maxCommonWords=0;
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        if((*lit)->mnLoopWords>maxCommonWords)
            maxCommonWords=(*lit)->mnLoopWords;
    }

    int minCommonWords = maxCommonWords*0.8f;

    int nscores=0;

    // Compute similarity score. Retain the matches whose score is higher than minScore
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;

        if(pKFi->mnLoopWords>minCommonWords)
        {
            nscores++;

            //float si = mpVoc->score_new(pKF->mBowVec,pKFi->mBowVec);
            float si = KeyFrame::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);

            pKFi->mLoopScore = si;
            if(si>=minScore)
                lScoreAndMatch.push_back(make_pair(si,pKFi));
        }
    }

    if(lScoreAndMatch.empty())
        return vector<boost::interprocess::offset_ptr<KeyFrame> >();

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lAccScoreAndMatch;
    float bestAccScore = minScore;

    // Lets now accumulate score by covisibility
    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lScoreAndMatch.begin(), itend=lScoreAndMatch.end(); it!=itend; it++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
        vector<boost::interprocess::offset_ptr<KeyFrame> > vpNeighs = pKFi->GetBestCovisibilityKeyFrames(10);

        float bestScore = it->first;
        float accScore = it->first;
        boost::interprocess::offset_ptr<KeyFrame>  pBestKF = pKFi;
        for(vector<boost::interprocess::offset_ptr<KeyFrame> >::iterator vit=vpNeighs.begin(), vend=vpNeighs.end(); vit!=vend; vit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKF2 = *vit;
            if(pKF2->mnLoopQuery==pKF->mnId && pKF2->mnLoopWords>minCommonWords)
            {
                accScore+=pKF2->mLoopScore;
                if(pKF2->mLoopScore>bestScore)
                {
                    pBestKF=pKF2;
                    bestScore = pKF2->mLoopScore;
                }
            }
        }

        lAccScoreAndMatch.push_back(make_pair(accScore,pBestKF));
        if(accScore>bestAccScore)
            bestAccScore=accScore;
    }

    // Return all those keyframes with a score higher than 0.75*bestScore
    float minScoreToRetain = 0.75f*bestAccScore;

    set<boost::interprocess::offset_ptr<KeyFrame> > spAlreadyAddedKF;
    vector<boost::interprocess::offset_ptr<KeyFrame> > vpLoopCandidates;
    vpLoopCandidates.reserve(lAccScoreAndMatch.size());

    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lAccScoreAndMatch.begin(), itend=lAccScoreAndMatch.end(); it!=itend; it++)
    {
        if(it->first>minScoreToRetain)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
            if(!spAlreadyAddedKF.count(pKFi))
            {
                vpLoopCandidates.push_back(pKFi);
                spAlreadyAddedKF.insert(pKFi);
            }
        }
    }


    return vpLoopCandidates;
}

void KeyFrameDatabase::DetectCandidates(boost::interprocess::offset_ptr<KeyFrame>  pKF, float minScore,vector<boost::interprocess::offset_ptr<KeyFrame> >& vpLoopCand, vector<boost::interprocess::offset_ptr<KeyFrame> >& vpMergeCand)
{
    set<boost::interprocess::offset_ptr<KeyFrame> > spConnectedKeyFrames = pKF->GetConnectedKeyFrames();
    list<boost::interprocess::offset_ptr<KeyFrame> > lKFsSharingWordsLoop,lKFsSharingWordsMerge;

    // Search all keyframes that share a word with current keyframes
    // Discard keyframes connected to the query keyframe
    {
        std::unique_lock<mutex> lock(mMutex);

        //for(DBoW2::BowVector::const_iterator vit=pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit != vend; vit++)
        for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++)
        {
            list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs = mvInvertedFile[vit->first];

            for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi=*lit;
                if(pKFi->GetMap()==pKF->GetMap()) // For consider a loop candidate it a candidate it must be in the same map
                {
                    if(pKFi->mnLoopQuery!=pKF->mnId)
                    {
                        pKFi->mnLoopWords=0;
                        if(!spConnectedKeyFrames.count(pKFi))
                        {
                            pKFi->mnLoopQuery=pKF->mnId;
                            lKFsSharingWordsLoop.push_back(pKFi);
                        }
                    }
                    pKFi->mnLoopWords++;
                }
                else if(!pKFi->GetMap()->IsBad())
                {
                    if(pKFi->mnMergeQuery!=pKF->mnId)
                    {
                        pKFi->mnMergeWords=0;
                        if(!spConnectedKeyFrames.count(pKFi))
                        {
                            pKFi->mnMergeQuery=pKF->mnId;
                            lKFsSharingWordsMerge.push_back(pKFi);
                        }
                    }
                    pKFi->mnMergeWords++;
                }
            }
        }
    }

    if(lKFsSharingWordsLoop.empty() && lKFsSharingWordsMerge.empty())
        return;

    if(!lKFsSharingWordsLoop.empty())
    {
        list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lScoreAndMatch;

        // Only compare against those keyframes that share enough words
        int maxCommonWords=0;
        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWordsLoop.begin(), lend= lKFsSharingWordsLoop.end(); lit!=lend; lit++)
        {
            if((*lit)->mnLoopWords>maxCommonWords)
                maxCommonWords=(*lit)->mnLoopWords;
        }

        int minCommonWords = maxCommonWords*0.8f;

        int nscores=0;

        // Compute similarity score. Retain the matches whose score is higher than minScore
        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWordsLoop.begin(), lend= lKFsSharingWordsLoop.end(); lit!=lend; lit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;

            if(pKFi->mnLoopWords>minCommonWords)
            {
                nscores++;

                //float si = mpVoc->KeyFrameDatabase::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);
                float si = KeyFrame::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);

                pKFi->mLoopScore = si;
                if(si>=minScore)
                    lScoreAndMatch.push_back(make_pair(si,pKFi));
            }
        }

        if(!lScoreAndMatch.empty())
        {
            list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lAccScoreAndMatch;
            float bestAccScore = minScore;

            // Lets now accumulate score by covisibility
            for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lScoreAndMatch.begin(), itend=lScoreAndMatch.end(); it!=itend; it++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
                vector<boost::interprocess::offset_ptr<KeyFrame> > vpNeighs = pKFi->GetBestCovisibilityKeyFrames(10);

                float bestScore = it->first;
                float accScore = it->first;
                boost::interprocess::offset_ptr<KeyFrame>  pBestKF = pKFi;
                for(vector<boost::interprocess::offset_ptr<KeyFrame> >::iterator vit=vpNeighs.begin(), vend=vpNeighs.end(); vit!=vend; vit++)
                {
                    boost::interprocess::offset_ptr<KeyFrame>  pKF2 = *vit;
                    if(pKF2->mnLoopQuery==pKF->mnId && pKF2->mnLoopWords>minCommonWords)
                    {
                        accScore+=pKF2->mLoopScore;
                        if(pKF2->mLoopScore>bestScore)
                        {
                            pBestKF=pKF2;
                            bestScore = pKF2->mLoopScore;
                        }
                    }
                }

                lAccScoreAndMatch.push_back(make_pair(accScore,pBestKF));
                if(accScore>bestAccScore)
                    bestAccScore=accScore;
            }

            // Return all those keyframes with a score higher than 0.75*bestScore
            float minScoreToRetain = 0.75f*bestAccScore;

            set<boost::interprocess::offset_ptr<KeyFrame> > spAlreadyAddedKF;
            vpLoopCand.reserve(lAccScoreAndMatch.size());

            for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lAccScoreAndMatch.begin(), itend=lAccScoreAndMatch.end(); it!=itend; it++)
            {
                if(it->first>minScoreToRetain)
                {
                    boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
                    if(!spAlreadyAddedKF.count(pKFi))
                    {
                        vpLoopCand.push_back(pKFi);
                        spAlreadyAddedKF.insert(pKFi);
                    }
                }
            }
        }

    }

    if(!lKFsSharingWordsMerge.empty())
    {
        list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lScoreAndMatch;

        // Only compare against those keyframes that share enough words
        int maxCommonWords=0;
        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWordsMerge.begin(), lend=lKFsSharingWordsMerge.end(); lit!=lend; lit++)
        {
            if((*lit)->mnMergeWords>maxCommonWords)
                maxCommonWords=(*lit)->mnMergeWords;
        }

        int minCommonWords = maxCommonWords*0.8f;

        int nscores=0;

        // Compute similarity score. Retain the matches whose score is higher than minScore
        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWordsMerge.begin(), lend=lKFsSharingWordsMerge.end(); lit!=lend; lit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;

            if(pKFi->mnMergeWords>minCommonWords)
            {
                nscores++;

                //float si = mpVoc->KeyFrameDatabase::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);
                float si = KeyFrame::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);

                pKFi->mMergeScore = si;
                if(si>=minScore)
                    lScoreAndMatch.push_back(make_pair(si,pKFi));
            }
        }

        if(!lScoreAndMatch.empty())
        {
            list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lAccScoreAndMatch;
            float bestAccScore = minScore;

            // Lets now accumulate score by covisibility
            for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lScoreAndMatch.begin(), itend=lScoreAndMatch.end(); it!=itend; it++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
                vector<boost::interprocess::offset_ptr<KeyFrame> > vpNeighs = pKFi->GetBestCovisibilityKeyFrames(10);

                float bestScore = it->first;
                float accScore = it->first;
                boost::interprocess::offset_ptr<KeyFrame>  pBestKF = pKFi;
                for(vector<boost::interprocess::offset_ptr<KeyFrame> >::iterator vit=vpNeighs.begin(), vend=vpNeighs.end(); vit!=vend; vit++)
                {
                    boost::interprocess::offset_ptr<KeyFrame>  pKF2 = *vit;
                    if(pKF2->mnMergeQuery==pKF->mnId && pKF2->mnMergeWords>minCommonWords)
                    {
                        accScore+=pKF2->mMergeScore;
                        if(pKF2->mMergeScore>bestScore)
                        {
                            pBestKF=pKF2;
                            bestScore = pKF2->mMergeScore;
                        }
                    }
                }

                lAccScoreAndMatch.push_back(make_pair(accScore,pBestKF));
                if(accScore>bestAccScore)
                    bestAccScore=accScore;
            }

            // Return all those keyframes with a score higher than 0.75*bestScore
            float minScoreToRetain = 0.75f*bestAccScore;

            set<boost::interprocess::offset_ptr<KeyFrame> > spAlreadyAddedKF;
            vpMergeCand.reserve(lAccScoreAndMatch.size());

            for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lAccScoreAndMatch.begin(), itend=lAccScoreAndMatch.end(); it!=itend; it++)
            {
                if(it->first>minScoreToRetain)
                {
                    boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
                    if(!spAlreadyAddedKF.count(pKFi))
                    {
                        vpMergeCand.push_back(pKFi);
                        spAlreadyAddedKF.insert(pKFi);
                    }
                }
            }
        }

    }

    //for(DBoW2::BowVector::const_iterator vit=pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit != vend; vit++)
    for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++)
    {
        list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs = mvInvertedFile[vit->first];

        for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi=*lit;
            pKFi->mnLoopQuery=-1;
            pKFi->mnMergeQuery=-1;
        }
    }

}

void KeyFrameDatabase::DetectBestCandidates(boost::interprocess::offset_ptr<KeyFrame> pKF, vector<boost::interprocess::offset_ptr<KeyFrame> > &vpLoopCand, vector<boost::interprocess::offset_ptr<KeyFrame> > &vpMergeCand, int nMinWords)
{
    list<boost::interprocess::offset_ptr<KeyFrame> > lKFsSharingWords;
    set<boost::interprocess::offset_ptr<KeyFrame> > spConnectedKF;

    // Search all keyframes that share a word with current frame
    {
        std::unique_lock<mutex> lock(mMutex);

        spConnectedKF = pKF->GetConnectedKeyFrames();

        //for(DBoW2::BowVector::const_iterator vit=pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit != vend; vit++)
        for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++)
        {
            list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs =   mvInvertedFile[vit->first];

            for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi=*lit;
                if(spConnectedKF.find(pKFi) != spConnectedKF.end())
                {
                    continue;
                }
                if(pKFi->mnPlaceRecognitionQuery!=pKF->mnId)
                {
                    pKFi->mnPlaceRecognitionWords=0;
                    pKFi->mnPlaceRecognitionQuery=pKF->mnId;
                    lKFsSharingWords.push_back(pKFi);
                }
               pKFi->mnPlaceRecognitionWords++;

            }
        }
    }
    if(lKFsSharingWords.empty())
        return;

    // Only compare against those keyframes that share enough words
    int maxCommonWords=0;
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        if((*lit)->mnPlaceRecognitionWords>maxCommonWords)
            maxCommonWords=(*lit)->mnPlaceRecognitionWords;
    }

    int minCommonWords = maxCommonWords*0.8f;

    if(minCommonWords < nMinWords)
    {
        minCommonWords = nMinWords;
    }

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lScoreAndMatch;

    int nscores=0;

    // Compute similarity score.
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;

        if(pKFi->mnPlaceRecognitionWords>minCommonWords)
        {
            nscores++;
            float si = KeyFrame::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);
            pKFi->mPlaceRecognitionScore=si;
            lScoreAndMatch.push_back(make_pair(si,pKFi));
        }
    }

    if(lScoreAndMatch.empty())
        return;

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lAccScoreAndMatch;
    float bestAccScore = 0;

    // Lets now accumulate score by covisibility
    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lScoreAndMatch.begin(), itend=lScoreAndMatch.end(); it!=itend; it++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
        vector<boost::interprocess::offset_ptr<KeyFrame> > vpNeighs = pKFi->GetBestCovisibilityKeyFrames(10);

        float bestScore = it->first;
        float accScore = bestScore;
        boost::interprocess::offset_ptr<KeyFrame>  pBestKF = pKFi;
        for(vector<boost::interprocess::offset_ptr<KeyFrame> >::iterator vit=vpNeighs.begin(), vend=vpNeighs.end(); vit!=vend; vit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKF2 = *vit;
            if(pKF2->mnPlaceRecognitionQuery!=pKF->mnId)
                continue;

            accScore+=pKF2->mPlaceRecognitionScore;
            if(pKF2->mPlaceRecognitionScore>bestScore)
            {
                pBestKF=pKF2;
                bestScore = pKF2->mPlaceRecognitionScore;
            }

        }
        lAccScoreAndMatch.push_back(make_pair(accScore,pBestKF));
        if(accScore>bestAccScore)
            bestAccScore=accScore;
    }

    // Return all those keyframes with a score higher than 0.75*bestScore
    float minScoreToRetain = 0.75f*bestAccScore;
    set<boost::interprocess::offset_ptr<KeyFrame> > spAlreadyAddedKF;
    vpLoopCand.reserve(lAccScoreAndMatch.size());
    vpMergeCand.reserve(lAccScoreAndMatch.size());
    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lAccScoreAndMatch.begin(), itend=lAccScoreAndMatch.end(); it!=itend; it++)
    {
        const float &si = it->first;
        if(si>minScoreToRetain)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
            if(!spAlreadyAddedKF.count(pKFi))
            {
                if(pKF->GetMap() == pKFi->GetMap())
                {
                    vpLoopCand.push_back(pKFi);
                }
                else
                {
                    vpMergeCand.push_back(pKFi);
                }
                spAlreadyAddedKF.insert(pKFi);
            }
        }
    }
}

bool compFirst(const pair<float, boost::interprocess::offset_ptr<KeyFrame> > & a, const pair<float, boost::interprocess::offset_ptr<KeyFrame> > & b)
{
    return a.first > b.first;
}


void KeyFrameDatabase::DetectNBestCandidates(boost::interprocess::offset_ptr<KeyFrame> pKF, vector<boost::interprocess::offset_ptr<KeyFrame> > &vpLoopCand, vector<boost::interprocess::offset_ptr<KeyFrame> > &vpMergeCand, int nNumCandidates)
{
    list<boost::interprocess::offset_ptr<KeyFrame> > lKFsSharingWords;
    set<boost::interprocess::offset_ptr<KeyFrame> > spConnectedKF;

    // Search all keyframes that share a word with current frame
    {
        std::unique_lock<mutex> lock(mMutex);

        spConnectedKF = pKF->GetConnectedKeyFrames();
        int counter = 0;

        //for(DBoW2::BowVector::const_iterator vit=pKF->mBowVec.begin(), vend=pKF->mBowVec.end(); vit != vend; vit++)
        for(auto vit= pKF->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++)
        {
            //std::cout<<"Counter: DetectNBestCandidates: "<<counter++<<std::endl;
            list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs =   mvInvertedFile[vit->first];

            for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi=*lit;
                if(pKFi->mnPlaceRecognitionQuery!=pKF->mnId)
                {
                    pKFi->mnPlaceRecognitionWords=0;
                    if(!spConnectedKF.count(pKFi))
                    {

                        pKFi->mnPlaceRecognitionQuery=pKF->mnId;
                        lKFsSharingWords.push_back(pKFi);
                    }
                }
                pKFi->mnPlaceRecognitionWords++;

            }
            if(counter++>=pKF->mBowVec->size())
                break;
        }
    }
    std::cout<<"DetectNBestCandidates: after loop\n";
    if(lKFsSharingWords.empty())
        return;

    // Only compare against those keyframes that share enough words
    int maxCommonWords=0;
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        if((*lit)->mnPlaceRecognitionWords>maxCommonWords)
            maxCommonWords=(*lit)->mnPlaceRecognitionWords;
    }

    int minCommonWords = maxCommonWords*0.8f;

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lScoreAndMatch;

    int nscores=0;

    // Compute similarity score.
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;

        if(pKFi->mnPlaceRecognitionWords>minCommonWords)
        {
            nscores++;
            float si = KeyFrame::score_KFDatabase_ptr(pKF->mBowVec,pKFi->mBowVec);
            pKFi->mPlaceRecognitionScore=si;
            lScoreAndMatch.push_back(make_pair(si,pKFi));
        }
    }

    if(lScoreAndMatch.empty())
        return;

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lAccScoreAndMatch;
    float bestAccScore = 0;

    // Lets now accumulate score by covisibility
    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lScoreAndMatch.begin(), itend=lScoreAndMatch.end(); it!=itend; it++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
        vector<boost::interprocess::offset_ptr<KeyFrame> > vpNeighs = pKFi->GetBestCovisibilityKeyFrames(10);

        float bestScore = it->first;
        float accScore = bestScore;
        boost::interprocess::offset_ptr<KeyFrame>  pBestKF = pKFi;
        for(vector<boost::interprocess::offset_ptr<KeyFrame> >::iterator vit=vpNeighs.begin(), vend=vpNeighs.end(); vit!=vend; vit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKF2 = *vit;
            if(pKF2->mnPlaceRecognitionQuery!=pKF->mnId)
                continue;

            accScore+=pKF2->mPlaceRecognitionScore;
            if(pKF2->mPlaceRecognitionScore>bestScore)
            {
                pBestKF=pKF2;
                bestScore = pKF2->mPlaceRecognitionScore;
            }

        }
        lAccScoreAndMatch.push_back(make_pair(accScore,pBestKF));
        if(accScore>bestAccScore)
            bestAccScore=accScore;
    }

    lAccScoreAndMatch.sort(compFirst);

    vpLoopCand.reserve(nNumCandidates);
    vpMergeCand.reserve(nNumCandidates);
    set<boost::interprocess::offset_ptr<KeyFrame> > spAlreadyAddedKF;
    int i = 0;
    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lAccScoreAndMatch.begin();
    while(i < lAccScoreAndMatch.size() && (vpLoopCand.size() < nNumCandidates || vpMergeCand.size() < nNumCandidates))
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
        if(pKFi->isBad())
            continue;

        if(!spAlreadyAddedKF.count(pKFi))
        {
            if(pKF->GetMap() == pKFi->GetMap() && vpLoopCand.size() < nNumCandidates)
            {
                vpLoopCand.push_back(pKFi);
            }
            else if(pKF->GetMap() != pKFi->GetMap() && vpMergeCand.size() < nNumCandidates && !pKFi->GetMap()->IsBad())
            {
                vpMergeCand.push_back(pKFi);
            }
            spAlreadyAddedKF.insert(pKFi);
        }
        i++;
        it++;
    }
}


vector<boost::interprocess::offset_ptr<KeyFrame> > KeyFrameDatabase::DetectRelocalizationCandidates(Frame *F, boost::interprocess::offset_ptr<Map>  pMap)
{
    list<boost::interprocess::offset_ptr<KeyFrame> > lKFsSharingWords;

    // Search all keyframes that share a word with current frame
    {
        std::unique_lock<mutex> lock(mMutex);

        for(DBoW2::BowVector::const_iterator vit=F->mBowVec.begin(), vend=F->mBowVec.end(); vit != vend; vit++){
        //for(auto vit= F->mBowVec->begin(); vit!=pKF->mBowVec->end(); vit++){
            list<boost::interprocess::offset_ptr<KeyFrame> > &lKFs =   mvInvertedFile[vit->first];

            for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
            {
                boost::interprocess::offset_ptr<KeyFrame>  pKFi=*lit;
                if(pKFi->mnRelocQuery!=F->mnId)
                {
                    pKFi->mnRelocWords=0;
                    pKFi->mnRelocQuery=F->mnId;
                    lKFsSharingWords.push_back(pKFi);
                }
                pKFi->mnRelocWords++;
            }
        }
    }
    if(lKFsSharingWords.empty())
        return vector<boost::interprocess::offset_ptr<KeyFrame> >();

    // Only compare against those keyframes that share enough words
    int maxCommonWords=0;
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        if((*lit)->mnRelocWords>maxCommonWords)
            maxCommonWords=(*lit)->mnRelocWords;
    }

    int minCommonWords = maxCommonWords*0.8f;

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lScoreAndMatch;

    int nscores=0;

    // Compute similarity score.
    for(list<boost::interprocess::offset_ptr<KeyFrame> >::iterator lit=lKFsSharingWords.begin(), lend= lKFsSharingWords.end(); lit!=lend; lit++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = *lit;
        std::map<double,unsigned int> temp_featmap;
        //insert the map from the frame.
        
         for(auto it = F->mBowVec.begin(); it != F->mBowVec.end(); ++it) {
            temp_featmap.insert(std::pair<double,unsigned int>(it->first,it->second));
         }
         


        if(pKFi->mnRelocWords>minCommonWords)
        {
            nscores++;
            //float si = mpVoc->score_new(F->mBowVec,pKFi->mBowVec);
            float si = pKFi->score_KFDatabase_frame(&temp_featmap,pKFi->mBowVec);
            pKFi->mRelocScore=si;
            lScoreAndMatch.push_back(make_pair(si,pKFi));
        }
    }

    if(lScoreAndMatch.empty())
        return vector<boost::interprocess::offset_ptr<KeyFrame> >();

    list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > > lAccScoreAndMatch;
    float bestAccScore = 0;

    // Lets now accumulate score by covisibility
    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lScoreAndMatch.begin(), itend=lScoreAndMatch.end(); it!=itend; it++)
    {
        boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
        vector<boost::interprocess::offset_ptr<KeyFrame> > vpNeighs = pKFi->GetBestCovisibilityKeyFrames(10);

        float bestScore = it->first;
        float accScore = bestScore;
        boost::interprocess::offset_ptr<KeyFrame>  pBestKF = pKFi;
        for(vector<boost::interprocess::offset_ptr<KeyFrame> >::iterator vit=vpNeighs.begin(), vend=vpNeighs.end(); vit!=vend; vit++)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKF2 = *vit;
            if(pKF2->mnRelocQuery!=F->mnId)
                continue;

            accScore+=pKF2->mRelocScore;
            if(pKF2->mRelocScore>bestScore)
            {
                pBestKF=pKF2;
                bestScore = pKF2->mRelocScore;
            }

        }
        lAccScoreAndMatch.push_back(make_pair(accScore,pBestKF));
        if(accScore>bestAccScore)
            bestAccScore=accScore;
    }

    // Return all those keyframes with a score higher than 0.75*bestScore
    float minScoreToRetain = 0.75f*bestAccScore;
    set<boost::interprocess::offset_ptr<KeyFrame> > spAlreadyAddedKF;
    vector<boost::interprocess::offset_ptr<KeyFrame> > vpRelocCandidates;
    vpRelocCandidates.reserve(lAccScoreAndMatch.size());
    for(list<pair<float,boost::interprocess::offset_ptr<KeyFrame> > >::iterator it=lAccScoreAndMatch.begin(), itend=lAccScoreAndMatch.end(); it!=itend; it++)
    {
        const float &si = it->first;
        if(si>minScoreToRetain)
        {
            boost::interprocess::offset_ptr<KeyFrame>  pKFi = it->second;
            if (pKFi->GetMap() != pMap)
                continue;
            if(!spAlreadyAddedKF.count(pKFi))
            {
                vpRelocCandidates.push_back(pKFi);
                spAlreadyAddedKF.insert(pKFi);
            }
        }
    }

    return vpRelocCandidates;
}

void KeyFrameDatabase::SetORBVocabulary(ORBVocabulary* pORBVoc)
{
    ORBVocabulary** ptr;
    ptr = (ORBVocabulary**)( &mpVoc );
    *ptr = pORBVoc;

    mvInvertedFile.clear();
    mvInvertedFile.resize(mpVoc->size());
}

} //namespace ORB_SLAM
