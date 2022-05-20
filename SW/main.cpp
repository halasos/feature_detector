#include <iostream>
#include <iostream>
#include "Types/log_messages.hpp"
#include "fast_detector/feature_detector.h"
using namespace std;

int main(int argc, char** argv)
{
    char select = '9';

        if (argc > 1) {
            select = *argv[1];
        }

        if (argc > 2) {
            hal_asos_demo::feature_detector::scene_img = std::string(argv[2]);
        }
        if(argc > 3){
            hal_asos_demo::feature_detector::block_size = atoi(argv[3]);
        }
        if(argc > 4){
            hal_asos_demo::feature_detector::th = atoi(argv[4]);
        }

        if(argc > 5){
                hal_asos_demo::feature_detector::image_ip = std::string(argv[5]);
        }
            switch (select) {
             #ifdef WITH_OPENCV2
             case 'o':
                hal_asos_demo::feature_detector::teste_feature_detector_opencv_sa();
                break;
             #endif
            case 'a':
               hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_SP();
               break;
            case 'b':
               hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_DP();
               break;
            case 'e':
                hal_asos_demo::feature_detector::test_feature_detector_edrosten_c_mframe();
                break;
             case 'n':
                hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_hls_net();
                break;
             case 'E':
                hal_asos_demo::feature_detector::test_feature_detector_edrosten_c_mframe_net();
                break;
              default:
                LOG_MSG << "Error arg1\n";
                break;
            }



    return 0;
}
