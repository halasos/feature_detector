#pragma once
#include <string>
namespace hal_asos_demo {
	namespace feature_detector {


		extern int block_size;
		extern std::string scene_img;
        extern std::string image_ip;
        extern int th;
        void test_fast_detector_std_alone_dual_master_DP(void);
        void test_fast_detector_std_alone_dual_master_SP(void);
        void test_feature_detector_edrosten_c_mframe(void);
        void teste_feature_detector_opencv_sa(void);

        void test_fast_detector_std_alone_dual(void);
        void test_feature_detector_edrosten_c_mframe_net(void);

        void test_fast_detector_std_alone_dual_master_net(void);
        void test_fast_detector_std_alone_dual_master_hls_net (void);

	}
}
