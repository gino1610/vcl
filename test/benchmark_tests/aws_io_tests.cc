#include <sys/stat.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <lz4.h>
#include <math.h>
#include <VCL.h>

#include "chrono/Chrono.h"

#include "tbb/concurrent_vector.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>


long long int get_folder_size(const std::string &path) {
    std::string cmd("du -sb ");
    cmd.append(path);
    cmd.append(" | cut -f1 2>&1");

    FILE *stream = popen(cmd.c_str(), "r");
    if (stream) {
        const int max_size = 256;
        char readbuf[max_size];
        if (fgets(readbuf, max_size, stream) != NULL) {
            return atoll(readbuf);
        }
        pclose(stream);
    }
    return -1;
}

std::string get_name(std::string &fullpath){
    size_t dir_ext = fullpath.find_last_of("/");
    std::string name = fullpath.substr(dir_ext + 1);
    size_t pos = name.find_last_of(".");
    name = name.substr(0, pos);
    return name;
}

VCL::CompressionType get_compression(int comp)
{
    switch (comp) {
        case 0:
            return VCL::CompressionType::NOCOMPRESSION;
        case 1:
            return VCL::CompressionType::GZIP;
        case 2:
            return VCL::CompressionType::ZSTD;
        case 3:
            return VCL::CompressionType::LZ4;
        case 4:
            return VCL::CompressionType::BLOSC;
        case 5:
            return VCL::CompressionType::BLZ4;
        case 6:
            return VCL::CompressionType::BLZ4HC;
        case 7:
            return VCL::CompressionType::BSNAPPY;
        case 8:
            return VCL::CompressionType::BZLIB;
        case 9:
            return VCL::CompressionType::BZSTD;
        case 10:
            return VCL::CompressionType::RLE;
    }
}

std::string compression_string(int comp)
{
    switch (comp) {
        case 0:
            return "NOCOMPRESSION";
        case 1:
            return "GZIP";
        case 2:
            return "ZSTD";
        case 3:
            return "LZ4";
        case 4:
            return "BLOSC";
        case 5:
            return "BLZ4";
        case 6:
            return "BLZ4HC";
        case 7:
            return "BSNAPPY";
        case 8:
            return "BZLIB";
        case 9:
            return "BZSTD";
        case 10:
            return "RLE";
    }
}

void get_file_sizes( std::vector<std::string> &tiff_files,
    std::vector<std::string> &tdb_files, 
    std::vector<std::vector<long long int>> &sizes, 
    bool remote_io, 
    VCL::RemoteConnection &remote)
{
    long long int img_size = 0;
    struct stat stat_buf;

    for ( int i = 0; i < tdb_files.size(); ++i ) {
        std::vector<long long int> size;

        if (remote_io) {
            size.push_back(remote.get_object_size(tiff_files[i]));
            size.push_back(remote.get_object_size(tdb_files[i]));
        }
        else {
            int irc = stat(tiff_files[i].c_str(), &stat_buf);
            if ( irc == 0 )
                img_size = stat_buf.st_size;
            
            size.push_back(img_size);
            size.push_back(get_folder_size(tdb_files[i]));
        }

        sizes.push_back(size);
    }
}

// void write(std::string &output_dir, std::string &basename, cv::Mat &cv_img,
//     std::vector<std::string> &tiff_files,
//     std::vector<std::string> &tdb_files,
//     std::vector<int> &heights, std::vector<int> &widths,
//     std::vector<std::vector<float>> &times,
//     int compression, int minimum, 
//     bool remote_io, 
//     VCL::RemoteConnection &remote)
// {
//     int height = 0;
//     int width = 0;

//     ChronoCpu tdb_chrono("Write TDB");
//     ChronoCpu tif_chrono("Write Tiff");

//     std::string tif_outdir = output_dir + "image_results/tiff/";
//     std::string tdb_outdir = output_dir + "image_results/tdb/";

//     std::vector<float> time;
    
//     // get height and width
//     height = cv_img.rows;
//     width = cv_img.cols;
//     heights.push_back(height);
//     widths.push_back(width);
//     VCL::CompressionType comp = get_compression(compression);
    
//     // Determine the tdb image name
//     if (remote_io) {
//         VCL::Image tdbimg(cv_img);
//         tdbimg.set_connection(remote);
        
//         // std::string tdbname = tdbimg.create_unique(tdb_outdir, VCL::TDB);
//         // basename = get_name(tdbname);
//         std::string tdbname = tdb_outdir + basename + ".tdb";
    
//         std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//         std::string cv_name = tif_outdir + basename + ".tiff";
//         tiff_files.push_back(cv_name);

//         std::vector<unsigned char> data;
//         cv::imencode(".tiff", cv_img, data);
//         tif_chrono.tic();
//         remote.write(cv_name, data);
//         tif_chrono.tac();

//         // Write the TDB
//         std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//         tdbimg.set_compression(comp);
//         tdbimg.set_minimum_dimension(minimum);
//         tdb_files.push_back(tdbname);

//         tdb_chrono.tic();
//         tdbimg.store(tdbname, VCL::TDB);
//         tdb_chrono.tac();
//     }
//     else {
//         VCL::Image tdbimg(cv_img);
//         // std::string tdbname = tdbimg.create_unique(tdb_outdir, VCL::TDB);
//         // basename = get_name(tdbname);
//         std::string tdbname = tdb_outdir + basename + ".tdb";
    
//         std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//         std::string cv_name = tif_outdir + basename + ".tiff";
//         tiff_files.push_back(cv_name);

//         tif_chrono.tic();
//         cv::imwrite(cv_name, cv_img);
//         tif_chrono.tac();  
    
//         // Write the TDB
//         std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//         tdbimg.set_compression(comp);
//         tdbimg.set_minimum_dimension(minimum);
//         tdb_files.push_back(tdbname);

//         tdb_chrono.tic();
//         tdbimg.store(tdbname, VCL::TDB);
//         tdb_chrono.tac();   
//     }
    
//     time.push_back(tif_chrono.getLastTime_us() / 1000.0);
//     time.push_back(tdb_chrono.getLastTime_us() / 1000.0);

//     times.push_back(time);
// }

void write(std::string &output_dir, 
    std::string &base, 
    cv::Mat &cv_img,
    std::string &type,
    int compression, int minimum, 
    VCL::RemoteConnection &remote, 
    // tbb::concurrent_vector<tbb::concurrent_vector<float>> &times)
    std::vector<std::vector<float>> &times)
{
    // tbb::concurrent_vector<float> time;
    std::vector<float> time;

    ChronoCpu localchrono("Write Local");
    ChronoCpu remotechrono("Write Remote");
    
    std::string outdir = output_dir + "image_results/" + type + "/";

    std::string local_name = outdir + base + "." + type;
    std::string remote_name = "s3://irlcsrtdbtests/image_results/" + type + "/" + base + "." + type;

    if (type == "tdb") {
        VCL::CompressionType comp = get_compression(compression);
        VCL::Image tdbimg(cv_img);
        tdbimg.set_compression(comp);
        tdbimg.set_minimum_dimension(minimum);
        localchrono.tic();
        tdbimg.store(local_name, VCL::TDB);
        localchrono.tac();

        VCL::Image remotetdb(cv_img);
        remotetdb.set_connection(remote);
        remotetdb.set_compression(comp);
        remotetdb.set_minimum_dimension(minimum);
        remotechrono.tic();
        remotetdb.store(remote_name, VCL::TDB);
        remotechrono.tac();
    }
    else if (type == "tiff") {
        std::vector<unsigned char> data;
        cv::imencode(".tiff", cv_img, data);
        remotechrono.tic();
        remote.write(remote_name, data);
        remotechrono.tac();

        localchrono.tic();
        cv::imwrite(local_name, cv_img);
        localchrono.tac();
    }

    time.push_back(localchrono.getLastTime_us() / 1000.0);
    time.push_back(remotechrono.getLastTime_us() / 1000.0);

    times.push_back(time);
}

void write_all(std::string &output_dir, 
    std::string &base, 
    cv::Mat &cv_img,
    std::string &type,
    int compression, int minimum, 
    bool remote_io,
    VCL::RemoteConnection &remote)
{
    std::string outdir = output_dir + "image_results/" + type + "/";

    std::string name = outdir + base + "." + type;

    if (type == "tdb") {
        VCL::CompressionType comp = get_compression(compression);
        if (remote_io) {
            VCL::Image remotetdb(cv_img);
            remotetdb.set_connection(remote);
            remotetdb.set_compression(comp);
            remotetdb.set_minimum_dimension(minimum);
            remotetdb.store(name, VCL::TDB);
        }
        else {
            VCL::Image tdbimg(cv_img);
            tdbimg.set_compression(comp);
            tdbimg.set_minimum_dimension(minimum);
            tdbimg.store(name, VCL::TDB);
        }
    }
    else if (type == "tiff") {
        if (remote_io) {
            std::vector<unsigned char> data;
            cv::imencode(".tiff", cv_img, data);
            remote.write(name, data);
        }
        else
            cv::imwrite(name, cv_img);
    }
}


// void read( std::vector<std::string> &tiff_files,
//     std::vector<std::string> &tdb_files,
//     std::vector<int> &heights, std::vector<int> &widths,
//     std::vector<std::vector<float>> &times, 
//     bool remote_io, 
//     VCL::RemoteConnection &remote)
// {
//     ChronoCpu read_tdb("Read TDB");
//     ChronoCpu tdb_mat("Read TDB to CV");
//     ChronoCpu read_tiff("Read TIFF");

//     for (int i = 0; i < tdb_files.size(); ++i) {
//         // Read the tiff image
//         std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

//         if (remote_io) {
//             read_tiff.tic();
//             std::vector<char> imgdata = remote.read(tiff_files[i]);
//             if ( !imgdata.empty() )
//                 cv::Mat tif_img = cv::imdecode(cv::Mat(imgdata), cv::IMREAD_ANYCOLOR);
//             read_tiff.tac();

//             // Read the TDB image into a buffer
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

//             read_tdb.tic();
//             VCL::Image tdb_img(tdb_files[i], remote);

//             int size = tdb_img.get_raw_data_size();
//             unsigned char* buffer = new unsigned char[size];

//             tdb_img.get_raw_data(buffer, size);
//             read_tdb.tac();

//             delete [] buffer;

//             // Read the TDB image into a mat
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

//             tdb_mat.tic();
//             VCL::Image tdbimg(tdb_files[i], remote);
//             cv::Mat tdbmat = tdbimg.get_cvmat();
//             tdb_mat.tac();

//         }
//         else {
//             read_tiff.tic();
//             cv::Mat tif_img = cv::imread(tiff_files[i], cv::IMREAD_ANYCOLOR);
//             read_tiff.tac();

//             // Read the TDB image into a buffer
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

//             read_tdb.tic();
//             VCL::Image tdb_img(tdb_files[i]);

//             int size = tdb_img.get_raw_data_size();
//             unsigned char* buffer = new unsigned char[size];

//             tdb_img.get_raw_data(buffer, size);
//             read_tdb.tac();

//             delete [] buffer;

//             // Read the TDB image into a mat
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");

//             tdb_mat.tic();
//             VCL::Image tdbimg(tdb_files[i]);

//             cv::Mat tdbmat = tdbimg.get_cvmat();
//             tdb_mat.tac();
//         }

//         times[i].push_back(read_tiff.getLastTime_us() / 1000.0);
//         times[i].push_back(read_tdb.getLastTime_us() / 1000.0);
//         times[i].push_back(tdb_mat.getLastTime_us() / 1000.0);
//     }
// }

void read(std::string &output_dir, 
    std::string &base,
    int height, int width,
    std::string &type,
    VCL::RemoteConnection &remote, 
    std::vector<std::vector<float>> &times, 
    int index)
{
    ChronoCpu localchrono("Read Local");
    ChronoCpu remotechrono("Read Remote");
    
    std::string outdir = output_dir + "image_results/" + type + "/";

    std::string local_name = outdir + base + "." + type;
    std::string remote_name = "s3://irlcsrtdbtests/image_results/" + type + "/" + base + "." + type;

    if (type == "tdb") {
        VCL::Image tdbimg(local_name);
        localchrono.tic();
        cv::Mat limg = tdbimg.get_cvmat();
        localchrono.tac();

        VCL::Image remotetdb(remote_name, remote);
        remotechrono.tic();
        cv::Mat rimg = remotetdb.get_cvmat();
        remotechrono.tac();
    }
    else if (type == "tiff") {
        localchrono.tic();
        cv::Mat rimg = cv::imread(local_name, cv::IMREAD_ANYCOLOR);
        localchrono.tac();

        remotechrono.tic();
        std::vector<char> imgdata = remote.read(remote_name);
        if ( !imgdata.empty() )
            cv::Mat limg = cv::imdecode(cv::Mat(imgdata), cv::IMREAD_ANYCOLOR);
        remotechrono.tac();
    }

    times[index].push_back(localchrono.getLastTime_us() / 1000.0);
    times[index].push_back(remotechrono.getLastTime_us() / 1000.0);
}


// void crop(std::vector<std::string> &tiff_files,
//     std::vector<std::string> &tdb_files,
//     std::vector<int> &heights, std::vector<int> &widths,
//     std::vector<std::vector<float>> &times, 
//     bool remote_io, 
//     VCL::RemoteConnection &remote)
// {
//     ChronoCpu crop_tdb("Crop TDB");
//     ChronoCpu tdb_mat("Read to Mat and Crop TDB");
//     ChronoCpu tiff("Read and Crop TIFF");

//     // int start_x = 100;
//     // int start_y = 100;
//     int start_x = 2235;
//     int start_y = 1233;
//     int height = 211;
//     int width = 81;
//     // rectangle is x, y, width, height

//     for (int i = 0; i < tdb_files.size(); ++i) {
//         // int height = (int)(heights[i] / 6.0);
//         // int width = (int)(widths[i] / 6.0);

//         if (remote_io) {
//             // Crop the TDB 
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//             crop_tdb.tic();
//             VCL::Image tdb_img(tdb_files[i], remote);
//             tdb_img.crop(VCL::Rectangle(start_x, start_y, width, height));
//             int size = height * width * 3;
//             unsigned char* raw_buffer = new unsigned char[size];

//             tdb_img.get_raw_data(raw_buffer, size);
//             crop_tdb.tac();

//             delete [] raw_buffer;

//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//             tiff.tic();
//             std::vector<char> imgdata = remote.read(tiff_files[i]);
//             if ( !imgdata.empty() ) {
//                 cv::Mat tifcrop = cv::imdecode(cv::Mat(imgdata), cv::IMREAD_ANYCOLOR);
//                 cv::Mat croppedtif(tifcrop, VCL::Rectangle(start_x, start_y, width, height));
//             }
//             tiff.tac();

//             // Read to CV and Crop the TDB
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//             tdb_mat.tic();
//             VCL::Image img(tdb_files[i], remote);
//             img.crop(VCL::Rectangle(start_x, start_y, width, height));
//             cv::Mat crop_mat = img.get_cvmat();
//             tdb_mat.tac();
//         }
//         else {
//             // Crop the TDB 
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//             crop_tdb.tic();
//             VCL::Image tdb_img(tdb_files[i]);
//             tdb_img.crop(VCL::Rectangle(start_x, start_y, width, height));
//             int size = height * width * 3;
//             unsigned char* raw_buffer = new unsigned char[size];

//             tdb_img.get_raw_data(raw_buffer, size);
//             crop_tdb.tac();

//             delete [] raw_buffer;

//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//             tiff.tic();
//             cv::Mat tifcrop = cv::imread(tiff_files[i], cv::IMREAD_ANYCOLOR);
//             cv::Mat croppedtif(tifcrop, VCL::Rectangle(start_x, start_y, width, height));
//             tiff.tac();

//             // Read to CV and Crop the TDB
//             std::system("sync && echo 3 > /proc/sys/vm/drop_caches");
//             tdb_mat.tic();
//             VCL::Image img(tdb_files[i]);
//             img.crop(VCL::Rectangle(start_x, start_y, width, height));
//             cv::Mat crop_mat = img.get_cvmat();
//             tdb_mat.tac();
//         }

//         times[i].push_back(tiff.getLastTime_us() / 1000.0);
//         times[i].push_back(crop_tdb.getLastTime_us() / 1000.0);
//         times[i].push_back(tdb_mat.getLastTime_us() / 1000.0);
//     }
// }

void crop(std::string &output_dir, 
    std::string &base,
    int height, int width,
    std::string &type,
    VCL::RemoteConnection &remote, 
    std::vector<std::vector<float>> &times, 
    int index)
{
    ChronoCpu localchrono("Crop Local");
    ChronoCpu remotechrono("Crop Remote");

    int start_x = 2235;
    int start_y = 1233;
    int height = 211;
    int width = 81;
    
    std::string outdir = output_dir + "image_results/" + type + "/";

    std::string local_name = outdir + base + "." + type;
    std::string remote_name = "s3://irlcsrtdbtests/image_results/" + type + "/" + base + "." + type;

    if (type == "tdb") {
        VCL::Image tdbimg(local_name);
        localchrono.tic();
        tdbimg.crop(VCL::Rectangle(start_x, start_y, width, height));
        cv::Mat crop_mat = tdbimg.get_cvmat();
        localchrono.tac();

        VCL::Image remotetdb(remote_name, remote);
        remotechrono.tic();
        remotetdb.crop(VCL::Rectangle(start_x, start_y, width, height));
        cv::Mat rimg = remotetdb.get_cvmat();
        remotechrono.tac();
    }
    else if (type == "tiff") {
        localchrono.tic();
        cv::Mat limg = cv::imread(local_name, cv::IMREAD_ANYCOLOR);
        cv::Mat localcrop(limg, VCL::Rectangle(start_x, start_y, width, height));
        localchrono.tac();

        remotechrono.tic();
        std::vector<char> imgdata = remote.read(remote_name);
        if ( !imgdata.empty() ) {
            cv::Mat rimg = cv::imdecode(cv::Mat(imgdata), cv::IMREAD_ANYCOLOR);
            cv::Mat remotecrop(rimg, VCL::Rectangle(start_x, start_y, width, height));
        }
        remotechrono.tac();
    }

    times[index].push_back(localchrono.getLastTime_us() / 1000.0);
    times[index].push_back(remotechrono.getLastTime_us() / 1000.0);
}

int main(int argc, char** argv )
{
    if ( argc != 6 )
    {
        printf("Usage: image dir, compression type, min tiles, output directory, output file name, remoteIO \n");
        return -1;
    }

    std::string image_dir = argv[1];
    int compression = atoi(argv[2]);
    int min_tiles = atoi(argv[3]);
    std::string output_dir = argv[4];
    std::string output_file = argv[5];
    // std::string io = argv[6];
    // bool remote_io = false;
    // if (io == "true")
    //     remote_io = true;

    std::ofstream outfile(output_file);

    std::vector<std::string> cameras;
    std::vector<std::string> frames;

    // tbb::concurrent_vector<std::string> tiff_files;
    // tbb::concurrent_vector<std::string> tdb_files;
    // tbb::concurrent_vector<int> heights;
    // tbb::concurrent_vector<int> widths;

    // tbb::concurrent_vector<tbb::concurrent_vector<float>> times;
    // tbb::concurrent_vector< std::vector<long long int>> sizes;
    std::vector<std::string> tiff_files;
    std::vector<std::string> tdb_files;
    std::vector<int> heights;
    std::vector<int> widths;

    std::vector<std::vector<float>> tdb_times;
    std::vector<std::vector<float>> tiff_times;
    std::vector< std::vector<long long int>> sizes;

    cv::Mat cv_img;
    
    int total_frames = 10;
    int total_cameras = 10;

    for (int i = 0; i < total_frames; ++i)
    {
        std::string frame_num = std::to_string(i);
        int zeros = 4 - frame_num.length();
        std::string frame = "F";
        for (int x = 0; x < zeros; ++x)
            frame += "0";
        frame += frame_num;
        frames.push_back(frame);
    }

    for (int j = 0; j < total_cameras; ++j)
    {
        int cam = j + 1;
        std::string cam_num = std::to_string(cam);
        int zeros = 4 - cam_num.length();

        std::string camera = "";
        for (int x = 0; x < zeros; ++x)
            camera += "0";

        camera += cam_num;
        cameras.push_back(camera);
    }

    std::string extension = "tdb";
    // // ChronoCpu tdb("Write TDB");
    // // ChronoCpu tif("Write TIFF");


    std::cout << "Writing Individual\n";
    // #pragma omp parallel for schedule(dynamic) 
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {    
            std::string name = frames[i] + "_" + cameras[j];

            std::string fullpath = image_dir + frames[i] + "/ForReconstruction/" + cameras[j] + ".tif";
            std::cout << fullpath << std::endl;
            std::vector<char> data = remote_read.read(fullpath);
            if ( !data.empty() )
                cv_img = cv::imdecode(cv::Mat(data), cv::IMREAD_ANYCOLOR);

            extension = "tdb";
            write(output_dir, name, cv_img, extension, compression, min_tiles, remote_write, tdb_times);
            extension = "tiff";
            write(output_dir, name, cv_img, extension, compression, min_tiles, remote_write, tiff_times);
        }
    }

    // std::string extension = "tiff";

    // std::cout << "Writing All\n";
    // std::string fullpath = image_dir + frames[0] + "/ForReconstruction/" + cameras[0] + ".tif";
    // std::vector<char> data = remote_read.read(fullpath);
    // if ( !data.empty() )
    //     cv_img = cv::imdecode(cv::Mat(data), cv::IMREAD_ANYCOLOR);
    // // #pragma omp parallel for schedule(dynamic) 
    // ChronoCpu total("Write Total");
    // total.tic();
    // for (int i = 0; i < total_frames; ++i)
    // {
    //     for (int j = 0; j < total_cameras; ++j)
    //     {
    //         std::string name = frames[i] + "_" + cameras[j];
    //         write_all(output_dir, name, cv_img, extension, compression, min_tiles, remote_io, remote_write);
    //     }
    // }
    // total.tac();
    // float total_time = total.getLastTime_us() / 1000.0;
    // std::cout << total_frames * total_cameras << " frames per  " << total_time << "ms\n";
    // std::cout << (total_frames * total_cameras) / (total_time / 1000.0) << " frames per second\n"; 

    std::cout << "Reading\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // read(tiff_files, tdb_files, heights, widths, times, remote_io, remote_write);
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {    
            std::string name = frames[i] + "_" + cameras[j];

            int index = i + j;
            extension = "tdb";
            read(output_dir, name, extension, compression, min_tiles, remote_write, tdb_times, index);
            extension = "tiff";
            read(output_dir, name, extension, compression, min_tiles, remote_write, tiff_times, index);
        }
    }
    std::cout << "Cropping\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // crop(tiff_files, tdb_files, heights, widths, times, remote_io, remote_write);
    for (int i = 0; i < total_frames; ++i)
    {
        for (int j = 0; j < total_cameras; ++j)
        {    
            std::string name = frames[i] + "_" + cameras[j];

            int index = i + j;
            extension = "tdb";
            write(output_dir, name, extension, compression, min_tiles, remote_write, tdb_times, index);
            extension = "tiff";
            write(output_dir, name, extension, compression, min_tiles, remote_write, tiff_times, index);
        }
    }
    // std::cout << "Getting File Sizes\n";
    // system("sync && echo 3 > /proc/sys/vm/drop_caches");
    // get_file_sizes(tiff_files, tdb_files, sizes, remote_io, remote_write);
    std::cout << "Output to " << output_file << std::endl;

    outfile << "# Image Name, Frame/Camera, ";
    // outfile << "TIFF Size, TDB Size, "
    outfile << "TIFF Local Write, TDB Local Write, ";
    outfile << "TIFF Remote Write, TDB Remote Write, ";
    
    // outfile << "TIFF Read, TDB Read, TDB Read to Mat, ";
    // outfile << "TIFF ROI, TDB ROI, TDB ROI to Mat, ";
    outfile << "\n";

    for (int i = 0; i < frames.size(); ++i) {
        for (int j = 0; j < cameras.size(); ++j) {
            outfile << frames[i] << ", " << cameras[j] << ", ";
            // for (int k = 0; k < sizes[i + j].size(); ++k) {
            //     outfile << sizes[i + j][k] << ", ";
            // }
            for (int x = 0; x < tdb_times[i + j].size(); ++x) {
                outfile << tiff_times[i + j][x] << ", ";
                outfile << tdb_times[i + j][x] << ", ";
            }
            outfile << std::endl;
        }
    }

  return 0;
}