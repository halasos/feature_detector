#include "feature_detector.h"
#include "../Task/hal_asos.h"

#include "edrosten.h"
#include <fstream>
#include <sstream>
#include "../Types/log_messages.hpp"
#include "sw_fast_v1_00.h"
#include "sw_nms_v1_00.h"
#include "sw_fast_lite_v1_00.h"
#include "sw_nms_lite_v1_00.h"
#include <list>
#include"Frame.h"
#define BLOCK_LEN 131072

#include "nms9.h"
#define CORNER_LEN 256
#define FEATURE_THRESHOLD 30
int hal_asos_demo::feature_detector::th =FEATURE_THRESHOLD;
int hal_asos_demo::feature_detector::block_size =  BLOCK_LEN;
std::string hal_asos_demo::feature_detector::scene_img = "512_512_table_if10.pgm";
std::string hal_asos_demo::feature_detector::image_ip = "127.0.0.1";
#define IMAGE_PORT_NO 60000
#ifdef WITH_OPENCV2
#include <opencv2/features2d/features2d.hpp>
#include<opencv2/highgui/highgui.hpp>


hal_asos::TaskConfig_t TFeatureDetectorCV = { "FeatureDetectorCV0",
{ "Corners",CORNER_LEN,1,1 },
{ "Imagelines",BLOCK_LEN,1,1 },
{ 1,1,1,1 }
};
#endif

hal_asos::TaskConfig_t TFastSA = { "FastSA0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};

hal_asos::TaskConfig_t TNonmaxSA = { "NonmaxSA0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};
hal_asos::TaskConfig_t TNonmaxNT = { "MNonmaxNet0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};


hal_asos::TaskConfig_t TFastSP = { "FastSP0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};

hal_asos::TaskConfig_t TNonmaxSP = { "NonmaxSP0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};


hal_asos::TaskConfig_t TFastDP = { "FastDP0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};

hal_asos::TaskConfig_t TNonmaxDP = { "NonmaxDP0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};

hal_asos::TaskConfig_t TNonmaxDPNet = { "NonmaxDPNet0",
{ "",0 },
{ "",0 },
{ 4,1,1,1 }
};

void hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual(void)
{
    using namespace hal_asos;

    Task<HwTask, TFastSA, profile<proxy::StandardFileIO>, segment_len<(BLOCK_LEN << 1)>>* p_T1 = new Task<HwTask,TFastSA, profile<proxy::StandardFileIO>, segment_len<(BLOCK_LEN << 1)>>();
    Task<HwTask, TNonmaxSA, profile<proxy::StandardFileIO>>* p_T2 = new  Task<HwTask, TNonmaxSA, profile<proxy::StandardFileIO>>();


    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version, width, height;
    std::stringstream ss;

    std::shared_ptr<char[]> p_data = std::shared_ptr<char[]>(new char[16]);
    std::shared_ptr<StreamData> Conf = std::make_shared<StreamData>(p_data, 16);
    detector::config_words* p_config = (detector::config_words*)p_data.get();


    CFstream<std::ifstream> Input_file(scene_img.c_str());
    Input_file.set_flags(std::ios::in | std::ifstream::binary);

    CFstream<std::ofstream> Output_file("Corners.bin");
    Output_file.set_flags(std::ios::out |std::ios::trunc|std::ios::binary);

    if (Output_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening output file!\n";
        return;
    }

    if (Input_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening input file!\n";
        return;
    }// First line : version

    Input_file.get_line(delimiter);
    if (delimiter.compare("P5") != 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:Wrong file format or version\n";
        return;
    }

    Input_file.get_line(delimiter2);
    ss.str(delimiter2);
    ss >> p_config->image_width >> p_config->image_height;

    if (p_config->image_width > BLOCK_LEN) {
        LOG_MSG << "[" << __FUNCTION__ << "Topiclen is smaller than image width\n";
        return;
    }

    //third line
    Input_file.get_line(delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> p_config->threshould;

    p_config->threshould = th;
    p_config->block_len = block_size;

    p_T1->submit_to_pool(Input_file);
    p_T2->submit_to_pool(Output_file);

    p_T1->submit_data(Conf);
    p_T2->submit_data(Conf);

    p_T1->start();
    p_T2->start();

    p_T1->join();
    p_T2->join();


    //Output_file.close_file();
    //Input_file.close_file();

    delete p_T1;
    delete p_T2;
}

void hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_SP(void)
{
    using namespace hal_asos;

    Task<HwTask, TFastSP, profile<proxy::StandardFileIO>, segment_len<(BLOCK_LEN << 1)>>* p_T1 = new Task<HwTask,TFastSP, profile<proxy::StandardFileIO>, segment_len<(BLOCK_LEN << 1)>>();
    Task<HwTask, TNonmaxSP, profile<proxy::StandardFileIO>>* p_T2 = new  Task<HwTask, TNonmaxSP, profile<proxy::StandardFileIO>>();


    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version, width, height;
    std::stringstream ss;

    std::shared_ptr<char[]> p_data = std::shared_ptr<char[]>(new char[16]);
    std::shared_ptr<StreamData> Conf = std::make_shared<StreamData>(p_data, 16);
    detector::config_words* p_config = (detector::config_words*)p_data.get();


    CFstream<std::ifstream> Input_file(scene_img.c_str());
    Input_file.set_flags(std::ios::in | std::ifstream::binary);

    CFstream<std::ofstream> Output_file("Corners.bin");
    Output_file.set_flags(std::ios::out |std::ios::trunc|std::ios::binary);

    if (Output_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening output file!\n";
        return;
    }

    if (Input_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening input file!\n";
        return;
    }// First line : version

    Input_file.get_line(delimiter);
    if (delimiter.compare("P5") != 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:Wrong file format or version\n";
        return;
    }

    Input_file.get_line(delimiter2);
    ss.str(delimiter2);
    ss >> p_config->image_width >> p_config->image_height;

    if (p_config->image_width > BLOCK_LEN) {
        LOG_MSG << "[" << __FUNCTION__ << "Topiclen is smaller than image width\n";
        return;
    }

    //third line
    Input_file.get_line(delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> p_config->threshould;

    p_config->threshould = th;
    p_config->block_len = block_size;

    p_T1->submit_to_pool(Input_file);
    p_T2->submit_to_pool(Output_file);

    p_T1->submit_data(Conf);
    p_T2->submit_data(Conf);

    p_T1->start();
    p_T2->start();

    p_T1->join();
    p_T2->join();


    //Output_file.close_file();
    //Input_file.close_file();

    delete p_T1;
    delete p_T2;
}

void hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_DP(void)
{
    using namespace hal_asos;

    Task<HwTask, TFastDP, profile<proxy::StandardFileIO>, segment_len<(BLOCK_LEN << 1)>>* p_T1 = new Task<HwTask,TFastDP, profile<proxy::StandardFileIO>, segment_len<(BLOCK_LEN << 1)>>();
    Task<HwTask, TNonmaxDP, profile<proxy::StandardFileIO>>* p_T2 = new  Task<HwTask, TNonmaxDP, profile<proxy::StandardFileIO>>();


    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version, width, height;
    std::stringstream ss;

    std::shared_ptr<char[]> p_data = std::shared_ptr<char[]>(new char[16]);
    std::shared_ptr<StreamData> Conf = std::make_shared<StreamData>(p_data, 16);
    detector::config_words* p_config = (detector::config_words*)p_data.get();


    CFstream<std::ifstream> Input_file(scene_img.c_str());
    Input_file.set_flags(std::ios::in | std::ifstream::binary);

    CFstream<std::ofstream> Output_file("Corners.bin");
    Output_file.set_flags(std::ios::out |std::ios::trunc|std::ios::binary);

    if (Output_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening output file!\n";
        return;
    }

    if (Input_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening input file!\n";
        return;
    }// First line : version

    Input_file.get_line(delimiter);
    if (delimiter.compare("P5") != 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:Wrong file format or version\n";
        return;
    }

    Input_file.get_line(delimiter2);
    ss.str(delimiter2);
    ss >> p_config->image_width >> p_config->image_height;

    if (p_config->image_width > BLOCK_LEN) {
        LOG_MSG << "[" << __FUNCTION__ << "Topiclen is smaller than image width\n";
        return;
    }

    //third line
    Input_file.get_line(delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> p_config->threshould;

    p_config->threshould = th;
    p_config->block_len = block_size;

    p_T1->submit_to_pool(Input_file);
    p_T2->submit_to_pool(Output_file);

    p_T1->submit_data(Conf);
    p_T2->submit_data(Conf);

    p_T1->start();
    p_T2->start();

    p_T1->join();
    p_T2->join();


    //Output_file.close_file();
    //Input_file.close_file();

    delete p_T1;
    delete p_T2;
}


void hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_hls_net(void)
{
    using namespace hal_asos;
    bool status;
    FrameControl iFrame;
    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version, width, height;
    std::stringstream ss;
    int ret;
    hal_asos::networking::CSocket<hal_asos::networking::Client> Soc;
    Task<HwTask, TFastDP, segment_len<(BLOCK_LEN << 1)>>* p_T1 = new Task<HwTask, TFastDP, segment_len<(BLOCK_LEN << 1)>>();
    Task<HwTask, TNonmaxDPNet, segment_len<(BLOCK_LEN << 1)>>* p_T2 = new  Task<HwTask, TNonmaxDPNet, segment_len<(BLOCK_LEN << 1)>>();

    Soc.set_ip_address(hal_asos_demo::feature_detector::image_ip);
    Soc.set_sock_type(SOCK_STREAM);
    Soc.set_sock_family(AF_INET);
    Soc.set_sock_port(IMAGE_PORT_NO);
    status = Soc.open_connection();
    if (!status) {
        Soc.get_error_message(hal_asos_demo::feature_detector::image_ip);
        Soc.close_connection();
        LOG_MSG << hal_asos_demo::feature_detector::image_ip << "\n";
        return;
    }

    std::shared_ptr<char[]> p_data = std::shared_ptr<char[]>(new char[16]);
    std::shared_ptr<StreamData> Conf = std::make_shared<StreamData>(p_data, 16);
    detector::config_words* p_config = (detector::config_words*)p_data.get();

    iFrame.top = TOPSYMBOL;
    std::copy_n(hal_asos_demo::feature_detector::scene_img.c_str(), hal_asos_demo::feature_detector::scene_img.length(), iFrame.filename);
    iFrame.filename[hal_asos_demo::feature_detector::scene_img.length()] = 0;
    iFrame.th = hal_asos_demo::feature_detector::th;
    iFrame.block_len = block_size;
    iFrame.delimitor = CONTROLSYMBOL;
    Soc.safe_write((char*)&iFrame, sizeof(struct FrameControl));

    CFstream<std::ifstream> Input_file(scene_img.c_str());
    Input_file.set_flags(std::ios::in | std::ifstream::binary);


    if (Input_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening input file!\n";
        return;
    }// First line : version

    Input_file.get_line(delimiter);
    if (delimiter.compare("P5") != 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:Wrong file format or version\n";
        return;
    }

    Input_file.get_line(delimiter2);
    ss.str(delimiter2);
    ss >> p_config->image_width >> p_config->image_height;

    if (p_config->image_width > BLOCK_LEN) {
        LOG_MSG << "[" << __FUNCTION__ << "Topiclen is smaller than image width\n";
        return;
    }

    //third line
    Input_file.get_line(delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> p_config->threshould;

    p_config->threshould = hal_asos_demo::feature_detector::th;
    p_config->block_len = block_size;

    p_T1->submit_to_pool(Input_file);
    p_T2->submit_to_pool(Soc);

    p_T1->submit_data(Conf);
    p_T2->submit_data(Conf);

    p_T1->start();
    p_T2->start();

    p_T1->join();
    p_T2->join();
    Input_file.close_file();

    Soc.safe_read((char*)&ret, 4);
    Soc.close_connection();

    delete p_T1;
    delete p_T2;
}


void hal_asos_demo::feature_detector::test_fast_detector_std_alone_dual_master_net(void)
{
    using namespace hal_asos;
    bool status;
    FrameControl iFrame;
    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version, width, height;
    std::stringstream ss;
    int ret;
    hal_asos::networking::CSocket<hal_asos::networking::Client> Soc;
    Task<HwTask, TFastSA,segment_len<(BLOCK_LEN << 1)>>* p_T1 = new Task<HwTask, TFastSA,  segment_len<(BLOCK_LEN << 1)>>();
    Task<HwTask, TNonmaxNT,segment_len<(BLOCK_LEN << 1)>>* p_T2 = new  Task<HwTask, TNonmaxNT,segment_len<(BLOCK_LEN << 1)>>();

    Soc.set_ip_address(hal_asos_demo::feature_detector::image_ip);
    Soc.set_sock_type(SOCK_STREAM);
    Soc.set_sock_family(AF_INET);
    Soc.set_sock_port(IMAGE_PORT_NO);
    status = Soc.open_connection();
    if (!status) {
        Soc.get_error_message(hal_asos_demo::feature_detector::image_ip);
        Soc.close_connection();
        LOG_MSG << hal_asos_demo::feature_detector::image_ip << "\n";
        return;
    }

    std::shared_ptr<char[]> p_data = std::shared_ptr<char[]>(new char[16]);
    std::shared_ptr<StreamData> Conf = std::make_shared<StreamData>(p_data, 16);
    detector::config_words* p_config = (detector::config_words*)p_data.get();

    iFrame.top = TOPSYMBOL;
    std::copy_n(hal_asos_demo::feature_detector::scene_img.c_str(), hal_asos_demo::feature_detector::scene_img.length(), iFrame.filename);
    iFrame.filename[hal_asos_demo::feature_detector::scene_img.length()] = 0;
    iFrame.th = hal_asos_demo::feature_detector::th;
    iFrame.block_len = CORNER_LEN;
    iFrame.delimitor = CONTROLSYMBOL;
    Soc.safe_write((char*)&iFrame, sizeof(struct FrameControl));

    CFstream<std::ifstream> Input_file(scene_img.c_str());
    Input_file.set_flags(std::ios::in | std::ifstream::binary);


    if (Input_file.open_file() < 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:error opening input file!\n";
        return;
    }// First line : version

    Input_file.get_line(delimiter);
    if (delimiter.compare("P5") != 0) {
        LOG_MSG << "[" << __FUNCTION__ << "]:Wrong file format or version\n";
        return;
    }

    Input_file.get_line(delimiter2);
    ss.str(delimiter2);
    ss >> p_config->image_width >> p_config->image_height;

    if (p_config->image_width > BLOCK_LEN) {
        LOG_MSG << "[" << __FUNCTION__ << "Topiclen is smaller than image width\n";
        return;
    }

    //third line
    Input_file.get_line(delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> p_config->threshould;

    p_config->threshould = hal_asos_demo::feature_detector::th;
    p_config->block_len = block_size;

    p_T1->submit_to_pool(Input_file);
    p_T2->submit_to_pool(Soc);

    p_T1->submit_data(Conf);
    p_T2->submit_data(Conf);

    p_T1->start();
    p_T2->start();

    p_T1->join();
    p_T2->join();



    Input_file.close_file();
    Soc.safe_read((char*)&ret, 4);
    Soc.close_connection();
    delete p_T1;
    delete p_T2;
}


void hal_asos_demo::feature_detector::test_feature_detector_edrosten_c_mframe(void) {
    edrosten::xy* nonmax;
    std::string TaskName = "[EdRosten-C]";
    std::string ofile = "CornersEdRostenML.txt";
    int nonmax_len;
    int index_pos = 0, ret = 1, coord_i;
    std::ifstream input_file;
    std::ofstream CornersFile;
    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version;
    std::stringstream lines;
    int width, height, threshould, imlen;
    int file_pos , count_corners=0;
    std::stringstream ss;
    std::shared_ptr<uint16_t[]> p_coord;
    std::shared_ptr<char[]> p_buff;

    int Read_len, pixel_len = 0;

    input_file.open(scene_img.c_str(), std::ios::in | std::ifstream::binary);
    if (!input_file.is_open()) {
        LOG_MSG << TaskName << ":error opening input file!\n";
        return;
    }// First line : version
    getline(input_file, delimiter);

    if (delimiter.compare("P5") != 0) {
        LOG_MSG << TaskName << "Wrong file format or version\n";
        return;
    }



    // Second line : comment?

    // secondline: wxh
    getline(input_file, delimiter2);


    ss.str(delimiter2);
    ss >> width >> height;

    imlen = width * height;

    //third line
    getline(input_file, delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> threshould;
    p_buff = std::shared_ptr<char[]>(new char[imlen]);
    Read_len = imlen;
    index_pos = 0;
    file_pos = 0;
    input_file.read(p_buff.get(), imlen);
    Read_len = (int)input_file.gcount();
    file_pos = Read_len;
    while (Read_len > 0) {
        nonmax = fast9_detect_nonmax((const edrosten::byte*)p_buff.get(), width, height, width, th, &nonmax_len);
        if (nonmax_len) {
                p_coord = std::shared_ptr<uint16_t[]>(new uint16_t[nonmax_len << 1]);
                CornersFile.open(ofile, std::ios::out | std::ios::trunc | std::ios::binary);
                for (index_pos = 0, coord_i = 0; index_pos < nonmax_len; index_pos++, coord_i += 2) {
                    p_coord[coord_i] = (uint16_t)nonmax[index_pos].x;
                    p_coord[coord_i + 1] = (uint16_t)nonmax[index_pos].y;
                    count_corners++;
        }
        CornersFile.write((char*)p_coord.get(), nonmax_len << 2);
        free(nonmax);

    }

        input_file.read(p_buff.get(), imlen);
        Read_len = (int)input_file.gcount();
        file_pos += Read_len;
    }


    CornersFile.close();
    lines.str("");

    input_file.close();

    
    LOG_MSG << TaskName << ":finished!...(" << file_pos << ", " << count_corners << ")\n";// ->" << ofile <<"\n";
    return;
}


void hal_asos_demo::feature_detector::test_feature_detector_edrosten_c_mframe_net(void) {
    edrosten::xy* nonmax;
    std::string TaskName = "[EdRosten-C Network]";
    bool status;
    hal_asos::networking::CSocket<hal_asos::networking::Client> Soc;
    int nonmax_len;
    int index_pos = 0, ret = 1, coord_i;
    std::ifstream input_file;
    std::string s;
    std::string delimiter = "";
    std::string delimiter2 = "";
    std::string delimiter3 = "";
    std::string version;
    FrameControl iFrame;
    std::stringstream lines;
    int width, height, threshould, imlen;
    int file_pos , count_corners=0;
    std::stringstream ss;
    std::shared_ptr<uint16_t[]> p_coord;
    std::shared_ptr<char[]> p_buff;

    int Read_len, pixel_len = 0;

    Soc.set_ip_address(hal_asos_demo::feature_detector::image_ip);
    Soc.set_sock_type(SOCK_STREAM);
    Soc.set_sock_family(AF_INET);
    Soc.set_sock_port(IMAGE_PORT_NO);
    status = Soc.open_connection();
    if (!status) {
        Soc.get_error_message(hal_asos_demo::feature_detector::image_ip);
        Soc.close_connection();
        LOG_MSG << hal_asos_demo::feature_detector::image_ip << "\n";
        return;
    }

    iFrame.top = TOPSYMBOL;
    std::copy_n(hal_asos_demo::feature_detector::scene_img.c_str(), hal_asos_demo::feature_detector::scene_img.length(), iFrame.filename);
    iFrame.filename[hal_asos_demo::feature_detector::scene_img.length()] = 0;
    iFrame.th = hal_asos_demo::feature_detector::th;
    iFrame.block_len = CORNER_LEN;
    iFrame.delimitor = CONTROLSYMBOL;
    Soc.safe_write((char*)&iFrame, sizeof(struct FrameControl));

    input_file.open(scene_img.c_str(), std::ios::in | std::ifstream::binary);
    if (!input_file.is_open()) {
        LOG_MSG << TaskName << ":error opening input file!\n";
        return;
    }// First line : version
    getline(input_file, delimiter);

    if (delimiter.compare("P5") != 0) {
        LOG_MSG << TaskName << "Wrong file format or version\n";
        return;
    }



    // Second line : comment?

    // secondline: wxh
    getline(input_file, delimiter2);


    ss.str(delimiter2);
    ss >> width >> height;

    imlen = width * height;

    //third line
    getline(input_file, delimiter3);
    ss.clear();
    ss.str(delimiter3);
    ss >> threshould;
    p_buff = std::shared_ptr<char[]>(new char[imlen]);
    Read_len = imlen;
    index_pos = 0;
    file_pos = 0;
    input_file.read(p_buff.get(), imlen);
    Read_len = (int)input_file.gcount();
    file_pos = Read_len;
    while (Read_len > 0) {
        nonmax = fast9_detect_nonmax((const edrosten::byte*)p_buff.get(), width, height, width, th, &nonmax_len);
        if (nonmax_len) {
                p_coord = std::shared_ptr<uint16_t[]>(new uint16_t[nonmax_len << 1]);
                for (index_pos = 0, coord_i = 0; index_pos < nonmax_len; index_pos++, coord_i += 2) {
                    p_coord[coord_i] = (uint16_t)nonmax[index_pos].x;
                    p_coord[coord_i + 1] = (uint16_t)nonmax[index_pos].y;
                    count_corners++;
        }


        Soc.safe_write((char*)p_coord.get(), nonmax_len << 2);
        free(nonmax);

    }

        input_file.read(p_buff.get(), imlen);
        Read_len = (int)input_file.gcount();
        file_pos += Read_len;
    }



    lines.str("");

    input_file.close();
    Soc.safe_read((char*)&ret, 4);
    Soc.close_connection();

    LOG_MSG << TaskName << ":finished!...(" << file_pos << ", " << count_corners << ")\n";// ->" << ofile <<"\n";
    return;
}

