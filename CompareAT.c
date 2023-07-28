#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <thread>

#include <unistd.h> /*UNIX标准函数定义*/ //没有的话write、open、close、read会报错

int fd;

int open_uart()
{

    fd = open("/dev/ttyUSB2", O_RDWR | O_NOCTTY | O_NDELAY);
    printf("fd:%d\n", fd);
    if (fd == -1)
    {
        perror("open tty fail");
        return -1;
    }
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        perror("open , fcntl fail");
        return -1;
    }
    else
    {
        printf("tty is opening\n\n");
    }
    //    printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
    //    printf("fd is %d\n", fd);
    /*
        if(isatty(STDIN_FILENO))
        {
            printf("is a tty\n");
        }
        else
        {
            printf("not tty\n");
        }
    */
    return 0;
}

int set_uart()
{
    struct termios options;

    if (tcgetattr(fd, &options) != 0)
    {
        perror("get uart attr fail\n");
        return -1;
    }
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    options.c_cflag |= CLOCAL | CREAD;
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_iflag &= ~INPCK;
    options.c_cflag &= ~CSTOPB;
    options.c_oflag &= ~OPOST;
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 1;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("uart set attr fail\n");
        return -1;
    }

    return 0;
}

int send_uart(char *str)
{
    int len;
    printf("send cmd:\t%s\n", str);
    len = write(fd, str, strlen(str));
    printf("send_len=%d\n", len);
    return 0;
}

int read_uart(char *buf)
{

    int flag = 0;
    int i = 0;
    fd_set fs_read;
    struct timeval time;
    printf("fd:%d\n", fd);

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = 0;
    time.tv_usec = 0;

    int len;

    int nread;
    char buff[1024];

    int ret;

    while (select(fd + 1, &fs_read, 0, 0, &time) > 0)
    {
        int len;
        char tbuf[1024];
        len = read(fd, tbuf, sizeof(tbuf));
        tbuf[len] = '\0';
        printf("%s", tbuf);
        // 将at返回值添加到buf字符串结尾
        strncat(buf, tbuf, sizeof(tbuf));
        flag = 1;
        i++;
    }
    printf("read_uart buf=%s\n", buf);

    return 0;
}

// 写入文件
int writeTXT(char *buf, char *n)
{
    char filename[70] = "/home/pi/Desktop/indoor_data/2023.04.09/";
    // 将命令中的参数取前三个作为文件名
    strncat(filename, n, 3);
    strncat(filename, "csq.txt", 15);

    printf("filename:%s\n", filename);
    FILE *CSQ_file = fopen(filename, "a+");

    fputs(buf, CSQ_file);
    fclose(CSQ_file);
    return 0;
}

// 获取当前系统时间
int getTime(char *buf)
{
    char *buff;
    buff = (char *)malloc(sizeof(char) * 100);

    // 结构体中有两个元素，一个是秒，一个是微秒
    struct timeval thisTime;
    gettimeofday(&thisTime, NULL);

    // 得到的是微秒，取前三位要毫秒
    int msec = thisTime.tv_usec / 1000;
    // 将int型转为char数组
    sprintf(buff, "%d\n", msec);

    // 存储的是1970年到现在经过多少秒
    time_t time_ptr;
    struct tm *nowTime = NULL;
    time(&time_ptr);
    // 年份得到的是1900年到现在的差值
    nowTime = localtime(&time_ptr);

    // printf("This time is %s\n",asctime(nowTime));
    // printf("This time is %d,%d,%d", (1900 + nowTime->tm_year), (1 + nowTime->tm_mon), (nowTime->tm_mday));
    // printf(" %d,%d,%d\n", nowTime->tm_hour, nowTime->tm_min, nowTime->tm_sec);

    // 将格式化的系统时间复制到buf中
    strcpy(buf, asctime(nowTime));
    strncat(buf, buff, sizeof(buff));

    // printf("Time buf:%s\n", buf);

    free(buff);
    return 0;
}

// chrono库获取到纳秒的时间
int show_time()
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();

    typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>>::type> Days; /* UTC: +8:00 */

    Days days = std::chrono::duration_cast<Days>(duration);
    duration -= days;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    duration -= milliseconds;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    duration -= microseconds;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

    std::cout << hours.count() << ":"
              << minutes.count() << ":"
              << seconds.count() << ":"
              << milliseconds.count() << ":"
              << microseconds.count() << ":"
              << nanoseconds.count() << std::endl;

    return 0;
}

int cmd_uart(char *str, char *n)
{
    // 用于计时
    auto start = std::chrono::high_resolution_clock::now();
    int stop_time = 15000;
    char *buf;
    buf = (char *)malloc(sizeof(char) * 1024);
    send_uart(str);
    getTime(buf);
    read_uart(buf);
    writeTXT(buf, n);
    free(buf);

    // show_time();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // 超过50000微秒就继续，少于就停下直到等于这个时间再继续
    if (elapsed_time >= stop_time)
    {
        std::cout << "elapsed_time > = 5    elapsed_time = " << elapsed_time << std::endl;
        start = end;
    }
    else
    {
        auto sleep_time = stop_time - elapsed_time - 55;
        std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
    }

    return 0;
}

int main(int argc, char **argv)
{
    char cmd_buf[1024];
    int go_flag = 1;

    open_uart();
    set_uart();

    // 命令后接参数,argv[0]是输入的命令，argv[1]开始才是参数
    char *point;
    point = argv[1];

    float secs = 3;
    clock_t delay = secs * CLOCKS_PER_SEC;
    clock_t start = clock();
    // 循环获取数据
    int flag = 1;

    // while (flag)
    // {
    //     if ((clock() - start) < delay)
    //     {
    //         cmd_uart("at+csq\r\n", point);
    //         cmd_uart("at+cpsi?\r\n", point);
    //     }
    //     else
    //     {
    //         flag = 0;
    //         printf("Done!\n");
    //         break;
    //     }
    //     // sleep(1);
    // }

    // 输入命令参数
    while (1)
    {
        cmd_uart("at+csq\r\n", point);
        // show_time();
        cmd_uart("at+cpsi?\r\n", point);
    }

    printf("Done!\n");
    close(fd);

    return 0;
}
