/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/9/9
 * @mail        : 396139919@qq.com
 * @project     : linux-tc-simulator
 * @file        : netCat.go
 * @description : TODO
 *******************************************************/

package netCat

import (
	"errors"
	"flag"
	"fmt"
	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
	"github.com/google/gopacket/pcap"
	"image"
	"image/color"
	"log"
	"net"
	"sync"
	"time"
)

var (
	downStreamDataSize = 0  // 单位时间内下行的总字节数
	upStreamDataSize   = 0  // 单位时间内上行的总字节数
	deviceName        = flag.String("i", "en0", "network interface device name") // 要监控的网卡名称
	locker sync.Mutex
	preTime = int64(0)
)

func NetCat() {
	flag.Parse()


	// Find all devices
	// 获取所有网卡
	devices, err := pcap.FindAllDevs()
	if err != nil {
		log.Fatal(err)
	}

	// Find exact device
	// 根据网卡名称从所有网卡中取到精确的网卡
	var device pcap.Interface
	for _, d := range devices {
		if d.Name == *deviceName {
			device = d
		}
	}

	// 根据网卡的ipv4地址获取网卡的mac地址，用于后面判断数据包的方向
	macAddr, err := findMacAddrByIp(findDeviceIpv4(device))
	if err != nil {
		panic(err)
	}

	fmt.Printf("Chosen device's IPv4: %s\n", findDeviceIpv4(device))
	fmt.Printf("Chosen device's MAC: %s\n", macAddr)

	// 获取网卡handler，可用于读取或写入数据包
	handle, err := pcap.OpenLive(*deviceName, 1024 /*每个数据包读取的最大值*/, true /*是否开启混杂模式*/, 400*time.Millisecond /*读包超时时长*/)
	if err != nil {
		panic(err)
	}
	defer handle.Close()

	// 开始抓包
	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())
	for packet := range packetSource.Packets() {
		// 获取所有以太网包
		ethernetLayer := packet.Layer(layers.LayerTypeEthernet)
		if ethernetLayer != nil {
			//ethernet := ethernetLayer.(*layers.Ethernet)
			// 如果封包的目的MAC是本机则表示是下行的数据包，否则为上行
			//if ethernet.DstMAC.String() == "f2:6b:3f:c7:ae:9b" {
			//	// 只统计udp
			//	udp := packet.Layer(layers.LayerTypeUDP)
			//	if udp != nil {
			//		// 统计下行封包总大小
			//		downStreamDataSize += len(packet.Data())*8
			//	}
			//}
			//if ethernet.SrcMAC.String() == "f2:6b:3f:c7:ae:9b"{
			//	// 只统计udp
			//	udp := packet.Layer(layers.LayerTypeUDP)
			//	if udp != nil {
			//		// 统计上行封包总大小
			locker.Lock()
					upStreamDataSize += len(packet.Data())*8
					fmt.Printf("test:%d\n",upStreamDataSize)
			locker.Unlock()
			//	}
			//}
		}
	}
}

// 获取网卡的IPv4地址
func findDeviceIpv4(device pcap.Interface) string {
	for _, addr := range device.Addresses {
		if ipv4 := addr.IP.To4(); ipv4 != nil {
			return ipv4.String()
		}
	}
	panic("device has no IPv4")
}

// 获取网卡的IPv6地址
func findDeviceIpv6(device pcap.Interface) string {
	for _, addr := range device.Addresses {
		if ipv6 := addr.IP.To16(); ipv6 != nil {
			return ipv6.String()
		}
	}
	panic("device has no IPv6")
}

// 根据网卡的IPv4地址获取MAC地址
// 有此方法是因为gopacket内部未封装获取MAC地址的方法，所以这里通过找到IPv4地址相同的网卡来寻找MAC地址
func findMacAddrByIp(ip string) (string, error) {
	interfaces, err := net.Interfaces()
	if err != nil {
		panic(interfaces)
	}

	for _, i := range interfaces {
		addrs, err := i.Addrs()
		if err != nil {
			panic(err)
		}

		for _, addr := range addrs {
			if a, ok := addr.(*net.IPNet); ok {
				if ip == a.IP.String() {
					return i.HardwareAddr.String(), nil
				}
			}
		}
	}
	return "", errors.New(fmt.Sprintf("no device has given ip: %s", ip))
}

// 每一秒计算一次该秒内的数据包大小平均值，并将下载、上传总量置零
func Monitor(x, yFix int, RBRate float32, gray *image.RGBA, now int64) {
	locker.Lock()
	if preTime == 0 {
		preTime = now
	} else {
		if now - preTime > 4000 {
			//fmt.Printf("\rDown:%.2fkb/s \t Up:%.2fkb/s \n", float32(downStreamDataSize)/1024/4, float32(upStreamDataSize)/1024/4)

			gray.SetRGBA(x, int(float32(upStreamDataSize)/1024*RBRate/4) + yFix/2,
				color.RGBA{255, 0,0,255})

			downStreamDataSize = 0
			upStreamDataSize = 0
			preTime = 0
		}
	}
	locker.Unlock()
}

