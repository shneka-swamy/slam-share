#!/bin/bash
pathDatasetEuroc='../../CollabAR/EuRoC/' #Example, it is necesary to change it by the dataset path

# Single Session Example (Pure visual)
echo "Launching MH01 with Stereo sensor"
../build/debug/bin/stereo_euroc ../Vocabulary/ORBvoc.txt ./Stereo/EuRoC.yaml "$pathDatasetEuroc"/machine_hall/MH_01 ./Stereo/EuRoC_TimeStamps/MH01.txt dataset-MH01_stereo
echo "------------------------------------"
echo "Evaluation of MH01 trajectory with Stereo sensor"
python ../evaluation/evaluate_ate_scale.py ../evaluation/Ground_truth/EuRoC_left_cam/MH01_GT.txt f_dataset-MH01_stereo.txt --plot MH01_stereo.pdf



# MultiSession Example (Pure visual)
echo "Launching Machine Hall with Stereo sensor"
../build/debug/bin/stereo_euroc ../Vocabulary/ORBvoc.txt ./Stereo/EuRoC.yaml "$pathDatasetEuroc"/machine_hall/MH_01 ./Stereo/EuRoC_TimeStamps/MH01.txt "$pathDatasetEuroc"/machine_hall/MH_02 ./Stereo/EuRoC_TimeStamps/MH02.txt "$pathDatasetEuroc"/machine_hall/MH_03 ./Stereo/EuRoC_TimeStamps/MH03.txt "$pathDatasetEuroc"/machine_hall/MH_04 ./Stereo/EuRoC_TimeStamps/MH04.txt "$pathDatasetEuroc"/machine_hall/MH_05 ./Stereo/EuRoC_TimeStamps/MH05.txt dataset-MH01_to_MH05_stereo
echo "------------------------------------"
echo "Evaluation of MAchine Hall trajectory with Stereo sensor"
python ../evaluation/evaluate_ate_scale.py ../evaluation/Ground_truth/EuRoC_left_cam/MH_GT.txt f_dataset-MH01_to_MH05_stereo.txt --plot MH01_to_MH05_stereo.pdf


# Single Session Example (Visual-Inertial)
echo "Launching V102 with Monocular-Inertial sensor"
../build/debug/bin/mono_inertial_euroc ../Vocabulary/ORBvoc.txt ./Monocular-Inertial/EuRoC.yaml "$pathDatasetEuroc"/vicon/V1_02 ./Monocular-Inertial/EuRoC_TimeStamps/V102.txt dataset-V102_monoi
echo "------------------------------------"
echo "Evaluation of V102 trajectory with Monocular-Inertial sensor"
python ../evaluation/evaluate_ate_scale.py "$pathDatasetEuroc"/vicon/V1_02/mav0/state_groundtruth_estimate0/data.csv f_dataset-V102_monoi.txt --plot V102_monoi.pdf


# MultiSession Monocular Examples

echo "Launching Vicon Room 2 with Monocular-Inertial sensor"
../build/debug/bin/mono_inertial_euroc ../Vocabulary/ORBvoc.txt ./Monocular-Inertial/EuRoC.yaml "$pathDatasetEuroc"/vicon/V2_01 ./Monocular-Inertial/EuRoC_TimeStamps/V201.txt "$pathDatasetEuroc"/vicon/V2_02 ./Monocular-Inertial/EuRoC_TimeStamps/V202.txt "$pathDatasetEuroc"/vicon/V2_03 ./Monocular-Inertial/EuRoC_TimeStamps/V203.txt dataset-V201_to_V203_monoi
echo "------------------------------------"
echo "Evaluation of Vicon Room 2 trajectory with Stereo sensor"
python ../evaluation/evaluate_ate_scale.py ../evaluation/Ground_truth/EuRoC_imu/V2_GT.txt f_dataset-V201_to_V203_monoi.txt --plot V201_to_V203_monoi.pdf

