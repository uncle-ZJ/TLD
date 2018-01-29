/**********************************************************
TLD测试代码
Leven
2016-01-24
本程序测试环境win7 64位，编译环境vs2013，OpenCV3.0.0，程序测试通过
感谢各路大神，程序基于alantrrs的版本，没有改动太大，修改了一下某些强制转换的地方，程序只能简单的运行起来
程序需要用命令行的方式调用，可以通过VS调试那里输入参数，也可以用我写的bat文件运行。
命令行参数是TLDtest2.exe  -p  parameters.yml
注意：debug比release速度慢，在vs里面运行比直接运行慢
**********************************************************/
#include <opencv2/opencv.hpp>
#include "tld_utils.h"
#include <iostream>
#include <sstream>
#include "TLD.h"
#include <stdio.h>
using namespace cv;
using namespace std;
//Global variables
Rect box;
bool drawing_box = false;
bool gotBB = false;
bool tl = true;
bool rep = false;
bool fromfile = false;
string video;
//读取记录bounding box的文件，获得bounding box的四个参数：左上角坐标x，y和宽高
/*如在\datasets\06_car\init.txt中：记录了初始目标的bounding box，内容如下
142,125,232,164
*/


void readBB(char* file)
{
	ifstream bb_file(file);							//ofstream是从内存到硬盘，ifstream是从硬盘到内存，其实所谓的流缓冲就是内存空间;
	string line;
	//istream& getline ( istream& , string& );
	//将输入流is中读到的字符存入str中，终结符默认为 '\n'（换行符） 

	getline(bb_file, line);							//http://blog.csdn.net/sunshine_in_moon/article/details/46638175?_t=t
	istringstream linestream(line);			//istringstream对象可以绑定一行字符串，然后以空格为分隔符把该行分隔开来。http://blog.sina.com.cn/s/blog_a9303fd90101adt6.html
	string x1, y1, x2, y2;
	//istream& getline ( istream &is , string &str , char delim ); 
	//将输入流is中读到的字符存入str中，直到遇到终结符delim才结束。

	getline(linestream, x1, ',');
	getline(linestream, y1, ',');
	getline(linestream, x2, ',');
	getline(linestream, y2, ',');
	//atoi 功 能： 把字符串转换成整型数
	int x = atoi(x1.c_str());// = (int)file["bb_x"];													//atoi (表示 ascii to integer)是把字符串转换成整型数的一个函数，https://baike.baidu.com/item/atoi/10931331?fr=aladdin
	int y = atoi(y1.c_str());// = (int)file["bb_y"];													//c_str它返回当前字符串的首字符地址
	int w = atoi(x2.c_str()) - x;// = (int)file["bb_w"];
	int h = atoi(y2.c_str()) - y;// = (int)file["bb_h"];
	box = Rect(x, y, w, h);
}
//bounding box mouse callback
//鼠标的响应就是得到目标区域的范围，用鼠标选中bounding box。

void mouseHandler(int event, int x, int y, int flags, void *param)
{
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:			//鼠标消息类型
		if (drawing_box)
		{
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:	//鼠标的坐标
		drawing_box = true;
		box = Rect(x, y, 0, 0);
		break;
	case CV_EVENT_LBUTTONUP:			//鼠标的坐标	
		drawing_box = false;
		if (box.width < 0) {
			box.x += box.width;
			box.width *= -1;
		}
		if (box.height < 0) {
			box.y += box.height;
			box.height *= -1;
		}
		gotBB = true;
		break;
	}
}

void print_help(char** argv) {
	printf("use:\n     %s -p /path/parameters.yml\n", argv[0]);
	printf("-s    source video\n-b        bounding box file\n-tl  track and learn\n-r     repeat\n");
}
//分析运行程序时的命令行参数
void read_options(int argc, char** argv, VideoCapture& capture, FileStorage &fs) {
	for (int i = 0; i<argc; i++)
	{
		if (strcmp(argv[i], "-b") == 0)//strcmp 比较两个字符串,  设这两个字符串为str1，str2， 若str1 == str2，则返回零；  若str1<str2，则返回负数；  若str1>str2，则返回正数。
		{
			if (argc>i)
			{
				readBB(argv[i + 1]);//是否指定初始的bounding box
				gotBB = true;
			}
			else
				print_help(argv);
		}
		if (strcmp(argv[i], "-s") == 0) { //从视频文件中读取
			if (argc>i)
			{
				video = string(argv[i + 1]);
				capture.open(video);
				fromfile = true;
			}
			else
				print_help(argv);

		}
		if (strcmp(argv[i], "-p") == 0) {
			if (argc>i)
			{
				fs.open(argv[i + 1], FileStorage::READ);
			}
			else
			{
				print_help(argv);
			}

		}
		if (strcmp(argv[i], "-no_tl") == 0)
		{
			tl = false;
		}
		if (strcmp(argv[i], "-r") == 0)
		{
			rep = true;
		}
	}
}



/*
运行程序时：
%To run from camera
./run_tld -p ../parameters.yml
%To run from file
./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg
%To init bounding box from file
./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt
%To train only in the first frame (no tracking, no learning)
./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt -no_tl
%To test the final detector (Repeat the video, first time learns, second time detects)
./run_tld -p ../parameters.yml -s ../datasets/06_car/car.mpg -b ../datasets/06_car/init.txt -r
*/
//感觉就是对起始帧进行初始化工作，然后逐帧读入图片序列，进行算法处理。


int main(int argc, char * argv[]) {
	VideoCapture capture;
	capture.open(1);
	//OpenCV的C++接口中，用于保存图像的imwrite只能保存整数数据，且需作为图像格式。当需要保存浮
	//点数据或XML/YML文件时，OpenCV的C语言接口提供了cvSave函数，但这一函数在C++接口中已经被删除。
	//取而代之的是FileStorage类。
	FileStorage fs;
	//Read options
	read_options(argc, argv, capture, fs);
	//Init camera
	if (!capture.isOpened()) { cout << "capture device failed to open!" << endl;  return 1; }
	//Register mouse callback to draw the bounding box
	namedWindow("TLD", CV_WINDOW_AUTOSIZE);
	setMouseCallback("TLD", mouseHandler, NULL);//用鼠标选中初始目标的bounding box //设置处理鼠标消息的回调函数http://blog.csdn.net/morewindows/article/details/8426283

												//第一个参数表示窗口名称。

												//第二个参数表示鼠标消息的消息处理函数。

												//第三个参数表示用户定义传入鼠标指定消息处理函数的参数。http://blog.csdn.net/morewindows/article/details/8426283
												//TLD framework
	TLD tld;
	//Read parameters file
	tld.read(fs.getFirstTopLevelNode());
	Mat frame;
	Mat last_gray;
	Mat first;
	if (fromfile) {																//如果指定为从文件读取
		capture >> frame;
		cvtColor(frame, last_gray, COLOR_RGB2GRAY);
		frame.copyTo(first);
	}
	else { //如果为读取摄像头，则设置获取的图像大小为320x240 
		capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	///Initialization
GETBOUNDINGBOX://标号：获取bounding box
	while (!gotBB)
	{
		if (!fromfile) {
			capture >> frame;
		}
		else
			first.copyTo(frame);
		cvtColor(frame, last_gray, COLOR_RGB2GRAY);
		drawBox(frame, box);													//把bounding box 画出来
		imshow("TLD", frame);
		if (waitKey(33) == 'q')
			return 0;
	}
	//由于图像片（min_win 为15x15像素）是在bounding box中采样得到的，所以box必须比min_win要大

	if (min(box.width, box.height)<(int)fs.getFirstTopLevelNode()["min_win"]) {
		cout << "Bounding box too small, try again." << endl;
		gotBB = false;
		goto GETBOUNDINGBOX;
	}
	//Remove callback
	setMouseCallback("TLD", NULL, NULL);						 //如果已经获得第一帧用户框定的box了，就取消鼠标响应
	printf("Initial Bounding Box = x:%d y:%d h:%d w:%d\n", box.x, box.y, box.width, box.height);
	//Output file
	FILE  *bb_file = fopen("bounding_boxes.txt", "w");			//fopen 返回值：文件顺利打开后，指向该流的文件指针就会被返回。如果文件打开失败则返回 NULL，并把错误代码存在 errno 中。
																//TLD initialization
	tld.init(last_gray, box, bb_file);											//https://zhidao.baidu.com/question/406141704.html

																				///Run-time
	Mat current_gray;
	BoundingBox pbox;
	vector<Point2f> pts1;
	vector<Point2f> pts2;
	bool status = true; //记录跟踪成功与否的状态 lastbox been found
	int frames = 1;//记录已过去帧数
	int detections = 1; //记录成功检测到的目标box数目
REPEAT:
	while (capture.read(frame)) {
		//get frame
		cvtColor(frame, current_gray, COLOR_RGB2GRAY);
		//Process Frame
		tld.processFrame(last_gray, current_gray, pts1, pts2, pbox, status, tl, bb_file);
		//Draw Points
		if (status) { //如果跟踪成功
			drawPoints(frame, pts1);
			drawPoints(frame, pts2, Scalar(0, 125, 0)); //当前的特征点用蓝色点表示
			drawBox(frame, pbox);
			detections++;		//记录成功检测到的目标box数目
		}
		//Display
		imshow("TLD", frame);
		//swap points and images
		swap(last_gray, current_gray);//STL函数swap()用来交换两对象的值。其泛型化版本定义于<algorithm>;改变指针指向的地址的值，即a和b的值互换了
		pts1.clear();//vector清空，内存释放
		pts2.clear();
		frames++; 						//记录已过去帧数
		printf("Detection rate: %d/%d\n", detections, frames);
		if (cvWaitKey(33) == 'q')
			break;
	}
	if (rep) 
	{
		rep = false;
		tl = false;
		fclose(bb_file);//功能是关闭一个流。注意：使用fclose()函数就可以把缓冲区内最后剩余的数据输出到内核缓冲区，并释放文件指针和有关的缓冲区
		bb_file = fopen("final_detector.txt", "w");
		//capture.set(CV_CAP_PROP_POS_AVI_RATIO,0);
		capture.release();
		capture.open(video);
		goto REPEAT;
	}
	fclose(bb_file);
	return 0;
}
