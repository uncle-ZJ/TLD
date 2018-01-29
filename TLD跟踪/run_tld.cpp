/**********************************************************
TLD���Դ���
Leven
2016-01-24
��������Ի���win7 64λ�����뻷��vs2013��OpenCV3.0.0���������ͨ��
��л��·���񣬳������alantrrs�İ汾��û�иĶ�̫���޸���һ��ĳЩǿ��ת���ĵط�������ֻ�ܼ򵥵���������
������Ҫ�������еķ�ʽ���ã�����ͨ��VS�����������������Ҳ��������д��bat�ļ����С�
�����в�����TLDtest2.exe  -p  parameters.yml
ע�⣺debug��release�ٶ�������vs�������б�ֱ��������
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
//��ȡ��¼bounding box���ļ������bounding box���ĸ����������Ͻ�����x��y�Ϳ��
/*����\datasets\06_car\init.txt�У���¼�˳�ʼĿ���bounding box����������
142,125,232,164
*/


void readBB(char* file)
{
	ifstream bb_file(file);							//ofstream�Ǵ��ڴ浽Ӳ�̣�ifstream�Ǵ�Ӳ�̵��ڴ棬��ʵ��ν������������ڴ�ռ�;
	string line;
	//istream& getline ( istream& , string& );
	//��������is�ж������ַ�����str�У��ս��Ĭ��Ϊ '\n'�����з��� 

	getline(bb_file, line);							//http://blog.csdn.net/sunshine_in_moon/article/details/46638175?_t=t
	istringstream linestream(line);			//istringstream������԰�һ���ַ�����Ȼ���Կո�Ϊ�ָ����Ѹ��зָ�������http://blog.sina.com.cn/s/blog_a9303fd90101adt6.html
	string x1, y1, x2, y2;
	//istream& getline ( istream &is , string &str , char delim ); 
	//��������is�ж������ַ�����str�У�ֱ�������ս��delim�Ž�����

	getline(linestream, x1, ',');
	getline(linestream, y1, ',');
	getline(linestream, x2, ',');
	getline(linestream, y2, ',');
	//atoi �� �ܣ� ���ַ���ת����������
	int x = atoi(x1.c_str());// = (int)file["bb_x"];													//atoi (��ʾ ascii to integer)�ǰ��ַ���ת������������һ��������https://baike.baidu.com/item/atoi/10931331?fr=aladdin
	int y = atoi(y1.c_str());// = (int)file["bb_y"];													//c_str�����ص�ǰ�ַ��������ַ���ַ
	int w = atoi(x2.c_str()) - x;// = (int)file["bb_w"];
	int h = atoi(y2.c_str()) - y;// = (int)file["bb_h"];
	box = Rect(x, y, w, h);
}
//bounding box mouse callback
//������Ӧ���ǵõ�Ŀ������ķ�Χ�������ѡ��bounding box��

void mouseHandler(int event, int x, int y, int flags, void *param)
{
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:			//�����Ϣ����
		if (drawing_box)
		{
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:	//��������
		drawing_box = true;
		box = Rect(x, y, 0, 0);
		break;
	case CV_EVENT_LBUTTONUP:			//��������	
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
//�������г���ʱ�������в���
void read_options(int argc, char** argv, VideoCapture& capture, FileStorage &fs) {
	for (int i = 0; i<argc; i++)
	{
		if (strcmp(argv[i], "-b") == 0)//strcmp �Ƚ������ַ���,  ���������ַ���Ϊstr1��str2�� ��str1 == str2���򷵻��㣻  ��str1<str2���򷵻ظ�����  ��str1>str2���򷵻�������
		{
			if (argc>i)
			{
				readBB(argv[i + 1]);//�Ƿ�ָ����ʼ��bounding box
				gotBB = true;
			}
			else
				print_help(argv);
		}
		if (strcmp(argv[i], "-s") == 0) { //����Ƶ�ļ��ж�ȡ
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
���г���ʱ��
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
//�о����Ƕ���ʼ֡���г�ʼ��������Ȼ����֡����ͼƬ���У������㷨����


int main(int argc, char * argv[]) {
	VideoCapture capture;
	capture.open(1);
	//OpenCV��C++�ӿ��У����ڱ���ͼ���imwriteֻ�ܱ����������ݣ�������Ϊͼ���ʽ������Ҫ���渡
	//�����ݻ�XML/YML�ļ�ʱ��OpenCV��C���Խӿ��ṩ��cvSave����������һ������C++�ӿ����Ѿ���ɾ����
	//ȡ����֮����FileStorage�ࡣ
	FileStorage fs;
	//Read options
	read_options(argc, argv, capture, fs);
	//Init camera
	if (!capture.isOpened()) { cout << "capture device failed to open!" << endl;  return 1; }
	//Register mouse callback to draw the bounding box
	namedWindow("TLD", CV_WINDOW_AUTOSIZE);
	setMouseCallback("TLD", mouseHandler, NULL);//�����ѡ�г�ʼĿ���bounding box //���ô��������Ϣ�Ļص�����http://blog.csdn.net/morewindows/article/details/8426283

												//��һ��������ʾ�������ơ�

												//�ڶ���������ʾ�����Ϣ����Ϣ��������

												//������������ʾ�û����崫�����ָ����Ϣ�������Ĳ�����http://blog.csdn.net/morewindows/article/details/8426283
												//TLD framework
	TLD tld;
	//Read parameters file
	tld.read(fs.getFirstTopLevelNode());
	Mat frame;
	Mat last_gray;
	Mat first;
	if (fromfile) {																//���ָ��Ϊ���ļ���ȡ
		capture >> frame;
		cvtColor(frame, last_gray, COLOR_RGB2GRAY);
		frame.copyTo(first);
	}
	else { //���Ϊ��ȡ����ͷ�������û�ȡ��ͼ���СΪ320x240 
		capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}

	///Initialization
GETBOUNDINGBOX://��ţ���ȡbounding box
	while (!gotBB)
	{
		if (!fromfile) {
			capture >> frame;
		}
		else
			first.copyTo(frame);
		cvtColor(frame, last_gray, COLOR_RGB2GRAY);
		drawBox(frame, box);													//��bounding box ������
		imshow("TLD", frame);
		if (waitKey(33) == 'q')
			return 0;
	}
	//����ͼ��Ƭ��min_win Ϊ15x15���أ�����bounding box�в����õ��ģ�����box�����min_winҪ��

	if (min(box.width, box.height)<(int)fs.getFirstTopLevelNode()["min_win"]) {
		cout << "Bounding box too small, try again." << endl;
		gotBB = false;
		goto GETBOUNDINGBOX;
	}
	//Remove callback
	setMouseCallback("TLD", NULL, NULL);						 //����Ѿ���õ�һ֡�û��򶨵�box�ˣ���ȡ�������Ӧ
	printf("Initial Bounding Box = x:%d y:%d h:%d w:%d\n", box.x, box.y, box.width, box.height);
	//Output file
	FILE  *bb_file = fopen("bounding_boxes.txt", "w");			//fopen ����ֵ���ļ�˳���򿪺�ָ��������ļ�ָ��ͻᱻ���ء�����ļ���ʧ���򷵻� NULL�����Ѵ��������� errno �С�
																//TLD initialization
	tld.init(last_gray, box, bb_file);											//https://zhidao.baidu.com/question/406141704.html

																				///Run-time
	Mat current_gray;
	BoundingBox pbox;
	vector<Point2f> pts1;
	vector<Point2f> pts2;
	bool status = true; //��¼���ٳɹ�����״̬ lastbox been found
	int frames = 1;//��¼�ѹ�ȥ֡��
	int detections = 1; //��¼�ɹ���⵽��Ŀ��box��Ŀ
REPEAT:
	while (capture.read(frame)) {
		//get frame
		cvtColor(frame, current_gray, COLOR_RGB2GRAY);
		//Process Frame
		tld.processFrame(last_gray, current_gray, pts1, pts2, pbox, status, tl, bb_file);
		//Draw Points
		if (status) { //������ٳɹ�
			drawPoints(frame, pts1);
			drawPoints(frame, pts2, Scalar(0, 125, 0)); //��ǰ������������ɫ���ʾ
			drawBox(frame, pbox);
			detections++;		//��¼�ɹ���⵽��Ŀ��box��Ŀ
		}
		//Display
		imshow("TLD", frame);
		//swap points and images
		swap(last_gray, current_gray);//STL����swap()���������������ֵ���䷺�ͻ��汾������<algorithm>;�ı�ָ��ָ��ĵ�ַ��ֵ����a��b��ֵ������
		pts1.clear();//vector��գ��ڴ��ͷ�
		pts2.clear();
		frames++; 						//��¼�ѹ�ȥ֡��
		printf("Detection rate: %d/%d\n", detections, frames);
		if (cvWaitKey(33) == 'q')
			break;
	}
	if (rep) 
	{
		rep = false;
		tl = false;
		fclose(bb_file);//�����ǹر�һ������ע�⣺ʹ��fclose()�����Ϳ��԰ѻ����������ʣ�������������ں˻����������ͷ��ļ�ָ����йصĻ�����
		bb_file = fopen("final_detector.txt", "w");
		//capture.set(CV_CAP_PROP_POS_AVI_RATIO,0);
		capture.release();
		capture.open(video);
		goto REPEAT;
	}
	fclose(bb_file);
	return 0;
}
