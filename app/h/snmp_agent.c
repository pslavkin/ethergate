//archivo para el manejo de SNMP sobre Udp
#include <iosne64.h>
#include <State_Machine.h>
#include <Events.h>
#include <Udp_Phisical.h>
#include <Serial_Tx.h>
#include <debug.h>
#include <arp_cache.h>
#include <One_Wire_Transport.h>
#include <One_Wire_Network.h>
//#include <Two_Wire_Transport.h>
#include <Snmp_Agent.h>
#include <String.h>
#include <Tcp_Transport.h>
#include <Type_conversion.h>
#include <Flash_Session.h>
#include <Sockets.h>
#include <Sockets_Fifo.h>
#include <Digio.h>

static State 
	Parsing_Version[],
	Parsing_Community[],
	Parsing_Bulk_Request[],
	Parsing_Object_Name[],
	Parsing_Msg_Code_Or_SysDescr[];

#pragma section {Flash_UData}
unsigned char Community[20]="public\r\n";
unsigned int Object_Name_Length=9;
unsigned char Object_Name[2]		={0x2b,0x06};
unsigned char Object_Name_Var[10]	={0x01,0x02,0x01,0x21,0x01,0x02,0x07};
#pragma section const {}
State* Snmp_Agent_Sm;
//------------------------------------------------------------------------------
void				Init_Snmp_Agent		(void)	{Snmp_Agent_Sm=Parsing_Version;}
State**				Snmp_Agent		(void)	{return &Snmp_Agent_Sm;}
//------------------------------------------------------------------------------
struct Snmp_Header_Struct*  	Rx_Snmp_Header		(void)	{(struct Snmp_Header_Struct*)	Udp_Rx_Payload();}
struct Snmp_Msg_Struct*     	Rx_Snmp_Msg		(void)	{(struct Snmp_Msg_Struct*)   	(Rx_Snmp_Header()->Community+Rx_Snmp_Header()->Community_Length);}
struct Snmp_Object_Struct*     	Rx_Snmp_Object		(void)	{(struct Snmp_Object_Struct*)   (Rx_Snmp_Msg()->Request_Id+Rx_Snmp_Msg()->Request_Id_Length);}
struct Snmp_Value_Struct*	Rx_Snmp_Value		(void)	{(struct Snmp_Value_Struct*)   	(Rx_Snmp_Object()->Object_Name+Rx_Snmp_Object()->Object_Name_Length);}
//------------------------------------------------------------------------------
void	Snmp_Packet_Arrived		(void)	{Atomic_Send_Event(Rx_Snmp_Header()->Version,Snmp_Agent());Send_NVDebug_Snmp_Agent_Data2Serial(9,"Version\r\n");}
void 	Parse_Community			(void)  {Atomic_Send_Event(Rx_Snmp_Header()->Community_Length == Search_NLine_On_String(Community,sizeof(Community)-1) && String_Compare(Rx_Snmp_Header()->Community,Community,Rx_Snmp_Header()->Community_Length),Actual_Sm());Send_NVDebug_Snmp_Agent_Data2Serial(11,"Community\r\n");}
void 	Parse_Msg_Code			(void)  {Atomic_Send_Event(Rx_Snmp_Msg()->Msg_Code,Actual_Sm());Send_NVDebug_Snmp_Agent_Data2Serial(6,"Code\r\n");}
void 	Parse_Object_Name		(void)	{Atomic_Send_Event(Rx_Snmp_Object()->Object_Name_Length==(Object_Name_Length+1) && String_Compare(Rx_Snmp_Object()->Object_Name,Object_Name,Rx_Snmp_Object()->Object_Name_Length-1),Actual_Sm());Send_NVDebug_Snmp_Agent_Data2Serial(13,"Object Name\r\n");}
void	Parse_Msg_Code_Or_SysDescr	(void)	{Atomic_Send_Event((Rx_Snmp_Msg()->Msg_Code<<8)|Rx_Snmp_Object()->Object_Name[Rx_Snmp_Object()->Object_Name_Length-1],Actual_Sm());Send_NVDebug_Snmp_Agent_Data2Serial(20,"Msg Code | SysDescr\n");}
//-----------------------------------------------------------------------------
void Response_Int(unsigned char SysDescr,signed int Value)
{
 Add_Received_Ip2Arp_Cache();
 Send_NVDebug_Snmp_Agent_Data2Serial(14,"Snmp Response\n");;

 
 Rx_Snmp_Object()		->Object_Name_Length			=Object_Name_Length+1;	//el +1 del final es porque el OID que se graba en el equipo puede tener muchos 'hijos' de 1 byte. por que es el que se mueve en el bulk de hecho y se recibe como parametro aca...
 Rx_Snmp_Object()		->Object_Name[Object_Name_Length]	=SysDescr; 	//como ultimo valor pone el OID que piden...
 String_Copy(Object_Name,Rx_Snmp_Object()->Object_Name,Object_Name_Length);		//aca se copua todo el header del OID que faltaba...

 Rx_Snmp_Value()		->Value_Code				=0x02;	//codigo que corresponde a Integer...
 Rx_Snmp_Value()		->Value_Length				=2;	//solo mando integer de 2 bytes...
 *(signed int*)Rx_Snmp_Value()	->Value					=Value;

 Rx_Snmp_Object()		->Item_Code		=0x30;
 Rx_Snmp_Object()		->Item_Length		=Rx_Snmp_Object()->Object_Name_Length+2+Rx_Snmp_Value()->Value_Length+2;
 Rx_Snmp_Object()		->Binding_Code		=0x30;
 Rx_Snmp_Object()		->Binding_Length	=Rx_Snmp_Object()->Item_Length+2; 
 Rx_Snmp_Object()		->Error_Status		=0x00;

 Rx_Snmp_Msg()			->Msg_Code		=Get_Response_Event;
 Rx_Snmp_Msg()			->Msg_Length		=Rx_Snmp_Object()->Binding_Length+2+6 + Rx_Snmp_Msg()->Request_Id_Length+2;

 Rx_Snmp_Header()		->Message_Length	=Rx_Snmp_Msg()->Msg_Length+2+Rx_Snmp_Header()->Community_Length+2+3;


 Ans_Udp(Rx_Snmp_Header()->Message_Length+2,Udp_Rx_Payload());
 Restore_Emac(); 
}
void Response_Err(void)
{
 Add_Received_Ip2Arp_Cache();
 Send_NVDebug_Snmp_Agent_Data2Serial(18,"Snmp Response Err\n");;

 Rx_Snmp_Value()		->Value_Code				=0x05;	//codigo que corresponde a NO-SUCH-NAME
 Rx_Snmp_Value()		->Value_Length				=1;	
 Rx_Snmp_Value()		->Value[0]				=0;

 Rx_Snmp_Object()		->Item_Length		=Rx_Snmp_Object()->Object_Name_Length+2+Rx_Snmp_Value()->Value_Length+2;
 Rx_Snmp_Object()		->Binding_Length	=Rx_Snmp_Object()->Item_Length+2; 
 Rx_Snmp_Object()		->Error_Status		=0x02;

 Rx_Snmp_Msg()			->Msg_Code		=Get_Response_Event;
 Rx_Snmp_Msg()			->Msg_Length		=Rx_Snmp_Object()->Binding_Length+2+6 + Rx_Snmp_Msg()->Request_Id_Length+2;

 Rx_Snmp_Header()		->Message_Length	=Rx_Snmp_Msg()->Msg_Length+2+Rx_Snmp_Header()->Community_Length+2+3;

 Ans_Udp(Rx_Snmp_Header()->Message_Length+2,Udp_Rx_Payload());
 Restore_Emac(); 
}
//-----------------------------------------------------------------------------
void Response_SHT71_RH		(void)	{Response_Int(0,1);/*Response_Int(0,SHT71_Bin_RH());*/ Send_NVDebug_Snmp_Agent_Data2Serial(8,"SHT71_H\n");}
void Response_SHT71_T		(void)	{Response_Int(1,1);/*Response_Int(1,SHT71_Bin_T()); */ Send_NVDebug_Snmp_Agent_Data2Serial(8,"SHT71_T\n");}
void Response_No_Such_Name	(void)	{Response_Err();		 Send_NVDebug_Snmp_Agent_Data2Serial(13,"No Such Name\n");}
//------------------------------------------------------------------------------
void Response_All_One_Wire_T	(void)	
{
 unsigned int Event=Actual_Event();
 if((Event>>8)==Get_Next_Request_Event || (Event>>8)==Get_Bulk_Request_Event) Event++;
 Event&=0x00FF;
 if(On_Line_Nodes()>Event) 	Response_Int(Event,One_Wire_Bin(Event));
 else 				Response_No_Such_Name();
 Send_NVDebug_Snmp_Agent_Data2Serial(11,"One Wire T\n");
}
void Response_First_One_Wire_T(void)
{
 if(On_Line_Nodes()) 	Response_Int(0,One_Wire_Bin(0));
 else 			Response_No_Such_Name();
}
//------------------------------------------------------------------------------
void Print_Bad_Version		(void)	{Restore_Emac();Send_NVDebug_Snmp_Agent_Data2Serial(12,"Bad Version\n");}
void Print_Bad_Community	(void)	{Restore_Emac();Send_NVDebug_Snmp_Agent_Data2Serial(14,"Bad Community\n");}
//------------------------------------------------------------------------------
void Load_Oid4Tcp		(void)	
{
 Send_NLine_String2Socket_Fifo(12,"001.003.006.");
 Save_Data4Tcp(sizeof(Object_Name_Var)*4);	//123.456.1.2. etc como maximo 3 digitos mas punto por cada MIB
} 
void View_Loaded_Oid		(void)	
{
 Add_NLine_String2Socket_Fifo(12,"001.003.006.");
 Send_String_NLine2Socket_Fifo(Search_NLine_On_String(Socket_Rx_Buffer(),sizeof(Object_Name_Var)*4),Socket_Rx_Buffer());
}
void Save_Oid4Loaded		(void)	
{
 unsigned char Bin[sizeof(Object_Name_Var)],Bin_Length;
 Bin_Length=Dot_Separated_Bcd2Chars(Socket_Rx_Buffer(),sizeof(Object_Name_Var)*4,Bin,sizeof(Object_Name_Var));
 Save_String2Flash((unsigned int*)Object_Name_Var,(unsigned int*)Bin,(Bin_Length+1)/2);
 Save_Int2Flash(&Object_Name_Length,Bin_Length+2);
}
void View_Actual_Oid		(void)	
{
 unsigned char Buf[sizeof(Object_Name_Var)*4];
 Chars2Dot_Separated_Bcd(Object_Name_Var,Object_Name_Length-2,Buf);
 Add_NLine_String2Socket_Fifo(12,"001.003.006.");
 Send_String_NLine2Socket_Fifo((Object_Name_Length-2)*4-1,Buf);
}
//------------------------------------------------------------------------------------------
void Load_Community4Tcp		(void)	{Save_Data4Tcp(sizeof(Community));} 
void View_Loaded_Community	(void)	{Send_NLine_String_NLine2Socket_Fifo(Search_NLine_On_String(Socket_Rx_Buffer(),sizeof(Community)),Socket_Rx_Buffer());}
void Save_Community4Loaded	(void)	{Save_String2Flash((unsigned int*)Community,(unsigned int*)Socket_Rx_Buffer(),sizeof(Community)/2);}
void View_Actual_Community	(void)	{Send_NLine_String_NLine2Socket_Fifo(Search_NLine_On_String(Community,sizeof(Community)),Community);}
//------------------------------------------------------------------------------
const State Parsing_Version[] =
{
 0x00				,Parse_Community		,Parsing_Community,	//SNMP V1
 0x01				,Parse_Community		,Parsing_Community, 	//SNMP V2c
 ANY_Event			,Print_Bad_Version		,Parsing_Version,
};
const State Parsing_Community[] =
{
 0x01				,Parse_Object_Name		,Parsing_Object_Name,
 ANY_Event			,Print_Bad_Community		,Parsing_Version,
};
const State Parsing_Object_Name[] =
{
 0x01				,Parse_Msg_Code_Or_SysDescr	,Parsing_Msg_Code_Or_SysDescr,
 ANY_Event			,Parse_Msg_Code			,Parsing_Bulk_Request,		//sino coincide, se investiga si esta pidiendo un get o un getnext o un bulk requesta para saber si tirar no such name o el primero de la lista respectivamente
};
const State Parsing_Bulk_Request[] =
{
 Get_Next_Request_Event		,Response_First_One_Wire_T	,Parsing_Version,
 Get_Bulk_Request_Event		,Response_First_One_Wire_T	,Parsing_Version,
 ANY_Event			,Response_No_Such_Name		,Parsing_Version,
};
const State Parsing_Msg_Code_Or_SysDescr[] =
{
 ANY_Event			,Response_All_One_Wire_T	,Parsing_Version,
};
//------------------------------------------------------------------------------

