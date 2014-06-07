# *****************************
# 预定义
# *****************************
CC = g++
CFLAGS = -c -g 
INCLUDES =  -I/usr/include -I/usr/local/include -I. -I../../common `mysql_config --cflags`
LIBS = -L/usr/lib64 -L/usr/local/lib64 -lACE -ldl -lcurl -lGeoIP -ltbbmalloc -lcommon `mysql_config --libs`

# *****************************
# 变换规则
# *****************************
# Here are some rules for converting .cpp -> .o
.SUFFIXES: .cpp .o
.cpp.o:
	@$(CC) $(CFLAGS) ${INCLUDES} -o $*.o $*.cpp 
	@echo '----- '$*.cpp' is compiled ok!'

# Here are some rules for converting .c -> .o
.SUFFIXES: .c .o
.c.o:
	@$(CC) $(CFLAGS) -o $*.o $*.c 
	@echo '----- '$*.c' is compiled ok!'

default:all

# 主应用文件列表
OBJS =  AccountSvc.o \
	AdSvc.o \
	BDSvc.o \
	AisSvc.o \
	AreaSvc.o \
	ClientSocket.o \
	ClientSocketMgr.o \
	CompanySearch.o \
	CompanySvc.o \
	DispatchCal.o \
	DispatchSvc.o \
	FleetSvc.o \
	GpsSvc.o \
	main.o \
	MainConfig.o \
	MainSvc.o \
	Message.o \
	MessageService.o \
	MessengerSvc.o \
	MobileSvc.o \
	MonitorSvc.o \
	PhoneSvc.o \
	PortSvc.o \
	ShipAis.o \
	ScheduleSvc.o \
	ServerMgr.o \
	SmsMailSvc.o \
	TraceSvc.o \
	UserRegSvc.o \
	VesselSvc.o \
	VoyageSvc.o \
	WeatherSvc.o \
	WebSvc.o \
	EquipmentSvc.o\
	BerthSvc.o\
	OilSpillSvc.o\
	eMopCommDataSvr.o\
	eMOPMaterialsSvc.o\
	eMOPEquipSvc.o\
	eMOPUserMgrSvc.o\
	eMOPMaintenanceUtil.o\
	eMOPMaintenanceSvc.o\
	eMOPRepairSvc.o\
	SARSvc.o
       	
# 主应用程序
APP_NAME = ../bin/blmbusiness

all:$(APP_NAME) Makefile

$(APP_NAME):$(OBJS)
	$(CC) -rdynamic -g -o $(APP_NAME) $(OBJS) $(LIBS)

# 清理
clean:
	rm -rf *.o
	rm -rf $(APP_NAME)
