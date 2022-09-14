/*******************************************************
 * @author      : dog head
 * @modified    : 2022-08-05 15:13
 * @mail        : 396139919@qq.com
 * @file        : SinBandwidth.go
 * @idea        : vim
 * @description : TODO
 *******************************************************/

package main

import (
	"bytes"
	"fmt"
	"github.com/transport-demo/linux-tc-simulator/SinBandwidth/netCat"
	"image"
	"image/color"
	"image/png"
	"io/ioutil"
	"log"
	"math"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"
)


func TotalFlowByDevice(dev string) uint64 {
	devinfo, err := ioutil.ReadFile("/proc/net/dev")
	if err != nil {
		return 0
	}

	var receive int = -1
	var transmit int = -1

	var receive_bytes uint64
	var transmit_bytes uint64

	lines := strings.Split(string(devinfo), "\n")
	for _, line := range lines {
		//fmt.Println("line :", line)
		if strings.Contains(line, dev) {
			i := 0
			fields := strings.Split(line, ":")
			for _, field := range fields {
				if strings.Contains(field, dev) {
					i = 1
				} else {
					values := strings.Fields(field)
					for _, value := range values {
						//fmt.Println("value :", value)
						if receive == i {
							bytes, _ := strconv.ParseInt(value, 10, 64)
							receive_bytes = uint64(bytes)
						} else if transmit == i {
							bytes, _ := strconv.ParseInt(value, 10, 64)
							transmit_bytes = uint64(bytes)
						}
						i++
					}
				}
			}
		} else if strings.Contains(line, "face") {
			index := 0
			tag := false
			fields := strings.Split(line, "|")
			for _, field := range fields {
				if strings.Contains(field, "face") {
					index = 1
				} else if strings.Contains(field, "bytes") {
					values := strings.Fields(field)
					for _, value := range values {
						//fmt.Println("value :", value)
						if strings.Contains(value, "bytes") {
							if !tag {
								tag = true
								receive = index
							} else {
								transmit = index
							}
						}
						index++
					}
				}
			}
		}
	}
	//fmt.Println("receive :", receive)
	//fmt.Println("transmit :", transmit)
	//fmt.Println("receive_bytes :", receive_bytes)
	//fmt.Println("transmit_bytes :", transmit_bytes)

	return receive_bytes + transmit_bytes
}

func add_tc_dev(bitrate string) {
	cmdArguments := []string{"qdisc", "add", "dev", "wlp5s0", "root", "tbf", "rate", bitrate, "latency", "50ms", "burst", "15kb"}

	cmd := exec.Command("tc", cmdArguments...)
	fmt.Println(cmd.String())

	var out bytes.Buffer
	var stderr bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &stderr
	err := cmd.Run()
	if err != nil {
		fmt.Println(err.Error())
	}
	//fmt.Println(out.String())
	//fmt.Println(stderr.String())

}

func change_tc_dev(bitrate string) {
	cmdArguments := []string{"qdisc", "change", "dev", "wlp5s0", "root", "tbf", "rate", bitrate, "latency", "50ms", "burst", "10kb"}

	cmd := exec.Command("tc", cmdArguments...)
	fmt.Println(cmd.String())

	var out bytes.Buffer
	var stderr bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &stderr
	err := cmd.Run()
	if err != nil {
		fmt.Println(err.Error())
	}
	//fmt.Println(stderr.String())
	//fmt.Println(out.String())

}
func del_tc_dev() {
	cmdArguments := []string{"qdisc", "del", "dev", "wlp5s0", "root"}

	cmd := exec.Command("tc", cmdArguments...)
	fmt.Println(cmd.String())

	var out bytes.Buffer
	var stderr bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &stderr
	err := cmd.Run()
	if err != nil {
		fmt.Println(err.Error())
	}
	//fmt.Println(stderr.String())
	//fmt.Println(out.String())
}

func createRGBA(high, width int) *image.RGBA {
	// 带宽大小作高，削减幅度作宽
	pic := image.NewRGBA(image.Rect(0, 0, width, high))

	// 创建根据带宽值、和削减值创建每个像素
	for x := 0; x < width; x++ {
		for y := 0; y < high; y++ {
			// 填充为白色
			pic.SetRGBA(x, y, color.RGBA{255,255,255,255})
		}
	}
	return pic
}


func main() {

	// 统计网卡数据
	go netCat.NetCat()

	initBitrate := os.Args[1]

	// 带宽值
	bitrate, _ := strconv.ParseFloat(initBitrate, 64)
	//bitrate := 1200

	// 削减值（也作为计算图片宽度的值）
	const bandwidthReduce = 400
	// 周期
	cycle := 3
	// 纵坐标修正值
	yFix := bandwidthReduce/2
	// 横坐标修正值
	xFix := bandwidthReduce/2

	pic := createRGBA( bandwidthReduce + yFix , bandwidthReduce * cycle + xFix)


	for i:=0; i<cycle; i++ {
		// 从0到最大像素生成x坐标
		for x := 0; x < bandwidthReduce; x++ {

			// 让sin的值的范围在0~2Pi之间，并且从负数开始
			s := float64(x + bandwidthReduce/4) * 2 * math.Pi / bandwidthReduce

			// sin的幅度为一半的像素。向下偏移一半像素并翻转
			y := bandwidthReduce/2 - math.Sin(s)*bandwidthReduce/2

			//fmt.Println(float64(bitrate) - y)
			TotalFlowByDevice("wlp5s0")
			//bitrateStr := fmt.Sprintf("%fkbit", float64(bitrate) - y)
			//  _ = fmt.Sprintf("%fkbit", float64(bitrate) - y)
			//if x == 0 {
			//	add_tc_dev(bitrateStr)
			//} else {
			//	change_tc_dev(bitrateStr)
			//}
			time.Sleep(300*time.Millisecond)

			// 按周期偏移，并用黑色绘制sin轨迹
			pic.SetRGBA(i * bandwidthReduce + x + xFix/2, int(y) + yFix/2, color.RGBA{0,0,0,255})
			netCat.Monitor(i * bandwidthReduce + x + xFix/2, yFix, float32(bandwidthReduce)/float32(bitrate), pic, time.Now().UnixMilli())
		}
	}


	// 创建文件
	file, err := os.Create("sin.png")

	if err != nil {
		log.Fatal(err)
	}
	// 使用png格式将数据写入文件
	err = png.Encode(file, pic) //将image信息写入文件中
	if err != nil {
		fmt.Printf(err.Error())
	}

	// 关闭文件
	err = file.Close()
	if err != nil {
		fmt.Printf(err.Error())
	}
	del_tc_dev()
}