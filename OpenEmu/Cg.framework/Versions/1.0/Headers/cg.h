/*
 *
 * Copyright (c) 2002-2012, NVIDIA Corporation.
 *
 *
 *
 * NVIDIA Corporation("NVIDIA") supplies this software to you in consideration
 * of your agreement to the following terms, and your use, installation,
 * modification or redistribution of this NVIDIA software constitutes
 * acceptance of these terms.  If you do not agree with these terms, please do
 * not use, install, modify or redistribute this NVIDIA software.
 *
 *
 *
 * In consideration of your agreement to abide by the following terms, and
 * subject to these terms, NVIDIA grants you a personal, non-exclusive license,
 * under NVIDIA's copyrights in this original NVIDIA software (the "NVIDIA
 * Software"), to use, reproduce, modify and redistribute the NVIDIA
 * Software, with or without modifications, in source and/or binary forms;
 * provided that if you redistribute the NVIDIA Software, you must retain the
 * copyright notice of NVIDIA, this notice and the following text and
 * disclaimers in all such redistributions of the NVIDIA Software. Neither the
 * name, trademarks, service marks nor logos of NVIDIA Corporation may be used
 * to endorse or promote products derived from the NVIDIA Software without
 * specific prior written permission from NVIDIA.  Except as expressly stated
 * in this notice, no other rights or licenses express or implied, are granted
 * by NVIDIA herein, including but not limited to any patent rights that may be
 * infringed by your derivative works or by other works in which the NVIDIA
 * Software may be incorporated. No hardware is licensed hereunder.
 *
 *
 *
 * THE NVIDIA SOFTWARE IS BEING PROVIDED ON AN "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 * WITHOUT LIMITATION, WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT,
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR ITS USE AND OPERATION
 * EITHER ALONE OR IN COMBINATION WITH OTHER PRODUCTS.
 *
 *
 *
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * EXEMPLARY, CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOST
 * PROFITS; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) OR ARISING IN ANY WAY OUT OF THE USE,
 * REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE NVIDIA SOFTWARE,
 * HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING
 * NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF NVIDIA HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _cg_h
#define _cg_h

/*************************************************************************/
/*** CG Run-Time Library API                                           ***/
/*************************************************************************/

#define CG_VERSION_NUM                3100

#ifdef _WIN32
# ifndef APIENTRY /* From Win32's <windef.h> */
#  define CG_APIENTRY_DEFINED
#  if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__) || defined(__LCC__)
#   define APIENTRY    __stdcall
#  else
#   define APIENTRY
#  endif
# endif
# ifndef WINGDIAPI /* From Win32's <wingdi.h> and <winnt.h> */
#  define CG_WINGDIAPI_DEFINED
#  define WINGDIAPI __declspec(dllimport)
# endif
#endif /* _WIN32 */

/* Set up CG_API for Win32 dllexport or gcc visibility. */

#ifndef CG_API
# ifdef CG_EXPORTS
#  ifdef _WIN32
#   define CG_API __declspec(dllexport)
#  elif defined(__GNUC__) && __GNUC__>=4
#   define CG_API __attribute__ ((visibility("default")))
#  elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#   define CG_API __global
#  else
#   define CG_API
#  endif
# else
#  define CG_API
# endif
#endif

#ifndef CGENTRY
# ifdef _WIN32
#  define CGENTRY __cdecl
# else
#  define CGENTRY
# endif
#endif

/*************************************************************************/
/*** Data types and enumerants                                         ***/
/*************************************************************************/

typedef int CGbool;
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;
typedef struct _CGobj *CGobj;
typedef struct _CGbuffer *CGbuffer;
typedef struct _CGeffect *CGeffect;
typedef struct _CGtechnique *CGtechnique;
typedef struct _CGpass *CGpass;
typedef struct _CGstate *CGstate;
typedef struct _CGstateassignment *CGstateassignment;
typedef struct _CGannotation *CGannotation;
typedef void *CGhandle;

#define CG_FALSE ((CGbool)0)
#define CG_TRUE  ((CGbool)1)

typedef enum
{
  CG_BEHAVIOR_UNKNOWN = 0,
  CG_BEHAVIOR_LATEST  = 1,                /* latest behavior supported at runtime      */
  CG_BEHAVIOR_2200    = 1000,             /* default behavior                          */
  CG_BEHAVIOR_3000    = 2000,
  CG_BEHAVIOR_3100    = 3000,
  CG_BEHAVIOR_CURRENT = CG_BEHAVIOR_3100  /* latest behavior supported at compile time */
} CGbehavior;

typedef enum
{
  CG_UNKNOWN_TYPE      = 0,
  CG_ARRAY             = 2,
  CG_STRING            = 1135,
  CG_STRUCT            = 1,
  CG_TYPELESS_STRUCT   = 3,
  CG_TEXTURE           = 1137,
  CG_BUFFER            = 1319,
  CG_UNIFORMBUFFER     = 1320,
  CG_ADDRESS           = 1321,
  CG_PIXELSHADER_TYPE  = 1142,
  CG_PROGRAM_TYPE      = 1136,
  CG_VERTEXSHADER_TYPE = 1141,
  CG_SAMPLER           = 1143,
  CG_SAMPLER1D         = 1065,
  CG_SAMPLER1DARRAY    = 1138,
  CG_SAMPLER1DSHADOW   = 1313,
  CG_SAMPLER2D         = 1066,
  CG_SAMPLER2DARRAY    = 1139,
  CG_SAMPLER2DMS       = 1317, /* ARB_texture_multisample */
  CG_SAMPLER2DMSARRAY  = 1318, /* ARB_texture_multisample */
  CG_SAMPLER2DSHADOW   = 1314,
  CG_SAMPLER3D         = 1067,
  CG_SAMPLERBUF        = 1144,
  CG_SAMPLERCUBE       = 1069,
  CG_SAMPLERCUBEARRAY  = 1140,
  CG_SAMPLERRBUF       = 1316, /* NV_explicit_multisample */
  CG_SAMPLERRECT       = 1068,
  CG_SAMPLERRECTSHADOW = 1315,
  CG_TYPE_START_ENUM   = 1024,
  CG_BOOL              = 1114,
  CG_BOOL1             = 1115,
  CG_BOOL2             = 1116,
  CG_BOOL3             = 1117,
  CG_BOOL4             = 1118,
  CG_BOOL1x1           = 1119,
  CG_BOOL1x2           = 1120,
  CG_BOOL1x3           = 1121,
  CG_BOOL1x4           = 1122,
  CG_BOOL2x1           = 1123,
  CG_BOOL2x2           = 1124,
  CG_BOOL2x3           = 1125,
  CG_BOOL2x4           = 1126,
  CG_BOOL3x1           = 1127,
  CG_BOOL3x2           = 1128,
  CG_BOOL3x3           = 1129,
  CG_BOOL3x4           = 1130,
  CG_BOOL4x1           = 1131,
  CG_BOOL4x2           = 1132,
  CG_BOOL4x3           = 1133,
  CG_BOOL4x4           = 1134,
  CG_CHAR              = 1166,
  CG_CHAR1             = 1167,
  CG_CHAR2             = 1168,
  CG_CHAR3             = 1169,
  CG_CHAR4             = 1170,
  CG_CHAR1x1           = 1171,
  CG_CHAR1x2           = 1172,
  CG_CHAR1x3           = 1173,
  CG_CHAR1x4           = 1174,
  CG_CHAR2x1           = 1175,
  CG_CHAR2x2           = 1176,
  CG_CHAR2x3           = 1177,
  CG_CHAR2x4           = 1178,
  CG_CHAR3x1           = 1179,
  CG_CHAR3x2           = 1180,
  CG_CHAR3x3           = 1181,
  CG_CHAR3x4           = 1182,
  CG_CHAR4x1           = 1183,
  CG_CHAR4x2           = 1184,
  CG_CHAR4x3           = 1185,
  CG_CHAR4x4           = 1186,
  CG_DOUBLE            = 1145,
  CG_DOUBLE1           = 1146,
  CG_DOUBLE2           = 1147,
  CG_DOUBLE3           = 1148,
  CG_DOUBLE4           = 1149,
  CG_DOUBLE1x1         = 1150,
  CG_DOUBLE1x2         = 1151,
  CG_DOUBLE1x3         = 1152,
  CG_DOUBLE1x4         = 1153,
  CG_DOUBLE2x1         = 1154,
  CG_DOUBLE2x2         = 1155,
  CG_DOUBLE2x3         = 1156,
  CG_DOUBLE2x4         = 1157,
  CG_DOUBLE3x1         = 1158,
  CG_DOUBLE3x2         = 1159,
  CG_DOUBLE3x3         = 1160,
  CG_DOUBLE3x4         = 1161,
  CG_DOUBLE4x1         = 1162,
  CG_DOUBLE4x2         = 1163,
  CG_DOUBLE4x3         = 1164,
  CG_DOUBLE4x4         = 1165,
  CG_FIXED             = 1070,
  CG_FIXED1            = 1092,
  CG_FIXED2            = 1071,
  CG_FIXED3            = 1072,
  CG_FIXED4            = 1073,
  CG_FIXED1x1          = 1074,
  CG_FIXED1x2          = 1075,
  CG_FIXED1x3          = 1076,
  CG_FIXED1x4          = 1077,
  CG_FIXED2x1          = 1078,
  CG_FIXED2x2          = 1079,
  CG_FIXED2x3          = 1080,
  CG_FIXED2x4          = 1081,
  CG_FIXED3x1          = 1082,
  CG_FIXED3x2          = 1083,
  CG_FIXED3x3          = 1084,
  CG_FIXED3x4          = 1085,
  CG_FIXED4x1          = 1086,
  CG_FIXED4x2          = 1087,
  CG_FIXED4x3          = 1088,
  CG_FIXED4x4          = 1089,
  CG_FLOAT             = 1045,
  CG_FLOAT1            = 1091,
  CG_FLOAT2            = 1046,
  CG_FLOAT3            = 1047,
  CG_FLOAT4            = 1048,
  CG_FLOAT1x1          = 1049,
  CG_FLOAT1x2          = 1050,
  CG_FLOAT1x3          = 1051,
  CG_FLOAT1x4          = 1052,
  CG_FLOAT2x1          = 1053,
  CG_FLOAT2x2          = 1054,
  CG_FLOAT2x3          = 1055,
  CG_FLOAT2x4          = 1056,
  CG_FLOAT3x1          = 1057,
  CG_FLOAT3x2          = 1058,
  CG_FLOAT3x3          = 1059,
  CG_FLOAT3x4          = 1060,
  CG_FLOAT4x1          = 1061,
  CG_FLOAT4x2          = 1062,
  CG_FLOAT4x3          = 1063,
  CG_FLOAT4x4          = 1064,
  CG_HALF              = 1025,
  CG_HALF1             = 1090,
  CG_HALF2             = 1026,
  CG_HALF3             = 1027,
  CG_HALF4             = 1028,
  CG_HALF1x1           = 1029,
  CG_HALF1x2           = 1030,
  CG_HALF1x3           = 1031,
  CG_HALF1x4           = 1032,
  CG_HALF2x1           = 1033,
  CG_HALF2x2           = 1034,
  CG_HALF2x3           = 1035,
  CG_HALF2x4           = 1036,
  CG_HALF3x1           = 1037,
  CG_HALF3x2           = 1038,
  CG_HALF3x3           = 1039,
  CG_HALF3x4           = 1040,
  CG_HALF4x1           = 1041,
  CG_HALF4x2           = 1042,
  CG_HALF4x3           = 1043,
  CG_HALF4x4           = 1044,
  CG_INT               = 1093,
  CG_INT1              = 1094,
  CG_INT2              = 1095,
  CG_INT3              = 1096,
  CG_INT4              = 1097,
  CG_INT1x1            = 1098,
  CG_INT1x2            = 1099,
  CG_INT1x3            = 1100,
  CG_INT1x4            = 1101,
  CG_INT2x1            = 1102,
  CG_INT2x2            = 1103,
  CG_INT2x3            = 1104,
  CG_INT2x4            = 1105,
  CG_INT3x1            = 1106,
  CG_INT3x2            = 1107,
  CG_INT3x3            = 1108,
  CG_INT3x4            = 1109,
  CG_INT4x1            = 1110,
  CG_INT4x2            = 1111,
  CG_INT4x3            = 1112,
  CG_INT4x4            = 1113,
  CG_LONG              = 1271,
  CG_LONG1             = 1272,
  CG_LONG2             = 1273,
  CG_LONG3             = 1274,
  CG_LONG4             = 1275,
  CG_LONG1x1           = 1276,
  CG_LONG1x2           = 1277,
  CG_LONG1x3           = 1278,
  CG_LONG1x4           = 1279,
  CG_LONG2x1           = 1280,
  CG_LONG2x2           = 1281,
  CG_LONG2x3           = 1282,
  CG_LONG2x4           = 1283,
  CG_LONG3x1           = 1284,
  CG_LONG3x2           = 1285,
  CG_LONG3x3           = 1286,
  CG_LONG3x4           = 1287,
  CG_LONG4x1           = 1288,
  CG_LONG4x2           = 1289,
  CG_LONG4x3           = 1290,
  CG_LONG4x4           = 1291,
  CG_SHORT             = 1208,
  CG_SHORT1            = 1209,
  CG_SHORT2            = 1210,
  CG_SHORT3            = 1211,
  CG_SHORT4            = 1212,
  CG_SHORT1x1          = 1213,
  CG_SHORT1x2          = 1214,
  CG_SHORT1x3          = 1215,
  CG_SHORT1x4          = 1216,
  CG_SHORT2x1          = 1217,
  CG_SHORT2x2          = 1218,
  CG_SHORT2x3          = 1219,
  CG_SHORT2x4          = 1220,
  CG_SHORT3x1          = 1221,
  CG_SHORT3x2          = 1222,
  CG_SHORT3x3          = 1223,
  CG_SHORT3x4          = 1224,
  CG_SHORT4x1          = 1225,
  CG_SHORT4x2          = 1226,
  CG_SHORT4x3          = 1227,
  CG_SHORT4x4          = 1228,
  CG_UCHAR             = 1187,
  CG_UCHAR1            = 1188,
  CG_UCHAR2            = 1189,
  CG_UCHAR3            = 1190,
  CG_UCHAR4            = 1191,
  CG_UCHAR1x1          = 1192,
  CG_UCHAR1x2          = 1193,
  CG_UCHAR1x3          = 1194,
  CG_UCHAR1x4          = 1195,
  CG_UCHAR2x1          = 1196,
  CG_UCHAR2x2          = 1197,
  CG_UCHAR2x3          = 1198,
  CG_UCHAR2x4          = 1199,
  CG_UCHAR3x1          = 1200,
  CG_UCHAR3x2          = 1201,
  CG_UCHAR3x3          = 1202,
  CG_UCHAR3x4          = 1203,
  CG_UCHAR4x1          = 1204,
  CG_UCHAR4x2          = 1205,
  CG_UCHAR4x3          = 1206,
  CG_UCHAR4x4          = 1207,
  CG_UINT              = 1250,
  CG_UINT1             = 1251,
  CG_UINT2             = 1252,
  CG_UINT3             = 1253,
  CG_UINT4             = 1254,
  CG_UINT1x1           = 1255,
  CG_UINT1x2           = 1256,
  CG_UINT1x3           = 1257,
  CG_UINT1x4           = 1258,
  CG_UINT2x1           = 1259,
  CG_UINT2x2           = 1260,
  CG_UINT2x3           = 1261,
  CG_UINT2x4           = 1262,
  CG_UINT3x1           = 1263,
  CG_UINT3x2           = 1264,
  CG_UINT3x3           = 1265,
  CG_UINT3x4           = 1266,
  CG_UINT4x1           = 1267,
  CG_UINT4x2           = 1268,
  CG_UINT4x3           = 1269,
  CG_UINT4x4           = 1270,
  CG_ULONG             = 1292,
  CG_ULONG1            = 1293,
  CG_ULONG2            = 1294,
  CG_ULONG3            = 1295,
  CG_ULONG4            = 1296,
  CG_ULONG1x1          = 1297,
  CG_ULONG1x2          = 1298,
  CG_ULONG1x3          = 1299,
  CG_ULONG1x4          = 1300,
  CG_ULONG2x1          = 1301,
  CG_ULONG2x2          = 1302,
  CG_ULONG2x3          = 1303,
  CG_ULONG2x4          = 1304,
  CG_ULONG3x1          = 1305,
  CG_ULONG3x2          = 1306,
  CG_ULONG3x3          = 1307,
  CG_ULONG3x4          = 1308,
  CG_ULONG4x1          = 1309,
  CG_ULONG4x2          = 1310,
  CG_ULONG4x3          = 1311,
  CG_ULONG4x4          = 1312,
  CG_USHORT            = 1229,
  CG_USHORT1           = 1230,
  CG_USHORT2           = 1231,
  CG_USHORT3           = 1232,
  CG_USHORT4           = 1233,
  CG_USHORT1x1         = 1234,
  CG_USHORT1x2         = 1235,
  CG_USHORT1x3         = 1236,
  CG_USHORT1x4         = 1237,
  CG_USHORT2x1         = 1238,
  CG_USHORT2x2         = 1239,
  CG_USHORT2x3         = 1240,
  CG_USHORT2x4         = 1241,
  CG_USHORT3x1         = 1242,
  CG_USHORT3x2         = 1243,
  CG_USHORT3x3         = 1244,
  CG_USHORT3x4         = 1245,
  CG_USHORT4x1         = 1246,
  CG_USHORT4x2         = 1247,
  CG_USHORT4x3         = 1248,
  CG_USHORT4x4         = 1249
} CGtype;

typedef enum
{
  CG_TEXUNIT0              = 2048,
  CG_TEXUNIT1              = 2049,
  CG_TEXUNIT2              = 2050,
  CG_TEXUNIT3              = 2051,
  CG_TEXUNIT4              = 2052,
  CG_TEXUNIT5              = 2053,
  CG_TEXUNIT6              = 2054,
  CG_TEXUNIT7              = 2055,
  CG_TEXUNIT8              = 2056,
  CG_TEXUNIT9              = 2057,
  CG_TEXUNIT10             = 2058,
  CG_TEXUNIT11             = 2059,
  CG_TEXUNIT12             = 2060,
  CG_TEXUNIT13             = 2061,
  CG_TEXUNIT14             = 2062,
  CG_TEXUNIT15             = 2063,
  CG_TEXUNIT16             = 4624,
  CG_TEXUNIT17             = 4625,
  CG_TEXUNIT18             = 4626,
  CG_TEXUNIT19             = 4627,
  CG_TEXUNIT20             = 4628,
  CG_TEXUNIT21             = 4629,
  CG_TEXUNIT22             = 4630,
  CG_TEXUNIT23             = 4631,
  CG_TEXUNIT24             = 4632,
  CG_TEXUNIT25             = 4633,
  CG_TEXUNIT26             = 4634,
  CG_TEXUNIT27             = 4635,
  CG_TEXUNIT28             = 4636,
  CG_TEXUNIT29             = 4637,
  CG_TEXUNIT30             = 4638,
  CG_TEXUNIT31             = 4639,
  CG_BUFFER0               = 2064,
  CG_BUFFER1               = 2065,
  CG_BUFFER2               = 2066,
  CG_BUFFER3               = 2067,
  CG_BUFFER4               = 2068,
  CG_BUFFER5               = 2069,
  CG_BUFFER6               = 2070,
  CG_BUFFER7               = 2071,
  CG_BUFFER8               = 2072,
  CG_BUFFER9               = 2073,
  CG_BUFFER10              = 2074,
  CG_BUFFER11              = 2075,
  CG_ATTR0                 = 2113,
  CG_ATTR1                 = 2114,
  CG_ATTR2                 = 2115,
  CG_ATTR3                 = 2116,
  CG_ATTR4                 = 2117,
  CG_ATTR5                 = 2118,
  CG_ATTR6                 = 2119,
  CG_ATTR7                 = 2120,
  CG_ATTR8                 = 2121,
  CG_ATTR9                 = 2122,
  CG_ATTR10                = 2123,
  CG_ATTR11                = 2124,
  CG_ATTR12                = 2125,
  CG_ATTR13                = 2126,
  CG_ATTR14                = 2127,
  CG_ATTR15                = 2128,
  CG_C                     = 2178,
  CG_TEX0                  = 2179,
  CG_TEX1                  = 2180,
  CG_TEX2                  = 2181,
  CG_TEX3                  = 2192,
  CG_TEX4                  = 2193,
  CG_TEX5                  = 2194,
  CG_TEX6                  = 2195,
  CG_TEX7                  = 2196,
  CG_HPOS                  = 2243,
  CG_COL0                  = 2245,
  CG_COL1                  = 2246,
  CG_COL2                  = 2247,
  CG_COL3                  = 2248,
  CG_PSIZ                  = 2309,
  CG_CLP0                  = 2310,
  CG_CLP1                  = 2311,
  CG_CLP2                  = 2312,
  CG_CLP3                  = 2313,
  CG_CLP4                  = 2314,
  CG_CLP5                  = 2315,
  CG_WPOS                  = 2373,
  CG_POINTCOORD            = 2374,
  CG_POSITION0             = 2437,
  CG_POSITION1             = 2438,
  CG_POSITION2             = 2439,
  CG_POSITION3             = 2440,
  CG_POSITION4             = 2441,
  CG_POSITION5             = 2442,
  CG_POSITION6             = 2443,
  CG_POSITION7             = 2444,
  CG_POSITION8             = 2445,
  CG_POSITION9             = 2446,
  CG_POSITION10            = 2447,
  CG_POSITION11            = 2448,
  CG_POSITION12            = 2449,
  CG_POSITION13            = 2450,
  CG_POSITION14            = 2451,
  CG_POSITION15            = 2452,
  CG_DIFFUSE0              = 2501,
  CG_TANGENT0              = 2565,
  CG_TANGENT1              = 2566,
  CG_TANGENT2              = 2567,
  CG_TANGENT3              = 2568,
  CG_TANGENT4              = 2569,
  CG_TANGENT5              = 2570,
  CG_TANGENT6              = 2571,
  CG_TANGENT7              = 2572,
  CG_TANGENT8              = 2573,
  CG_TANGENT9              = 2574,
  CG_TANGENT10             = 2575,
  CG_TANGENT11             = 2576,
  CG_TANGENT12             = 2577,
  CG_TANGENT13             = 2578,
  CG_TANGENT14             = 2579,
  CG_TANGENT15             = 2580,
  CG_SPECULAR0             = 2629,
  CG_BLENDINDICES0         = 2693,
  CG_BLENDINDICES1         = 2694,
  CG_BLENDINDICES2         = 2695,
  CG_BLENDINDICES3         = 2696,
  CG_BLENDINDICES4         = 2697,
  CG_BLENDINDICES5         = 2698,
  CG_BLENDINDICES6         = 2699,
  CG_BLENDINDICES7         = 2700,
  CG_BLENDINDICES8         = 2701,
  CG_BLENDINDICES9         = 2702,
  CG_BLENDINDICES10        = 2703,
  CG_BLENDINDICES11        = 2704,
  CG_BLENDINDICES12        = 2705,
  CG_BLENDINDICES13        = 2706,
  CG_BLENDINDICES14        = 2707,
  CG_BLENDINDICES15        = 2708,
  CG_COLOR0                = 2757,
  CG_COLOR1                = 2758,
  CG_COLOR2                = 2759,
  CG_COLOR3                = 2760,
  CG_COLOR4                = 2761,
  CG_COLOR5                = 2762,
  CG_COLOR6                = 2763,
  CG_COLOR7                = 2764,
  CG_COLOR8                = 2765,
  CG_COLOR9                = 2766,
  CG_COLOR10               = 2767,
  CG_COLOR11               = 2768,
  CG_COLOR12               = 2769,
  CG_COLOR13               = 2770,
  CG_COLOR14               = 2771,
  CG_COLOR15               = 2772,
  CG_PSIZE0                = 2821,
  CG_PSIZE1                = 2822,
  CG_PSIZE2                = 2823,
  CG_PSIZE3                = 2824,
  CG_PSIZE4                = 2825,
  CG_PSIZE5                = 2826,
  CG_PSIZE6                = 2827,
  CG_PSIZE7                = 2828,
  CG_PSIZE8                = 2829,
  CG_PSIZE9                = 2830,
  CG_PSIZE10               = 2831,
  CG_PSIZE11               = 2832,
  CG_PSIZE12               = 2833,
  CG_PSIZE13               = 2834,
  CG_PSIZE14               = 2835,
  CG_PSIZE15               = 2836,
  CG_BINORMAL0             = 2885,
  CG_BINORMAL1             = 2886,
  CG_BINORMAL2             = 2887,
  CG_BINORMAL3             = 2888,
  CG_BINORMAL4             = 2889,
  CG_BINORMAL5             = 2890,
  CG_BINORMAL6             = 2891,
  CG_BINORMAL7             = 2892,
  CG_BINORMAL8             = 2893,
  CG_BINORMAL9             = 2894,
  CG_BINORMAL10            = 2895,
  CG_BINORMAL11            = 2896,
  CG_BINORMAL12            = 2897,
  CG_BINORMAL13            = 2898,
  CG_BINORMAL14            = 2899,
  CG_BINORMAL15            = 2900,
  CG_FOG0                  = 2917,
  CG_FOG1                  = 2918,
  CG_FOG2                  = 2919,
  CG_FOG3                  = 2920,
  CG_FOG4                  = 2921,
  CG_FOG5                  = 2922,
  CG_FOG6                  = 2923,
  CG_FOG7                  = 2924,
  CG_FOG8                  = 2925,
  CG_FOG9                  = 2926,
  CG_FOG10                 = 2927,
  CG_FOG11                 = 2928,
  CG_FOG12                 = 2929,
  CG_FOG13                 = 2930,
  CG_FOG14                 = 2931,
  CG_FOG15                 = 2932,
  CG_DEPTH0                = 2933,
  CG_DEPTH1                = 2934,
  CG_DEPTH2                = 2935,
  CG_DEPTH3                = 2936,
  CG_DEPTH4                = 2937,
  CG_DEPTH5                = 2938,
  CG_DEPTH6                = 2939,
  CG_DEPTH7                = 2940,
  CG_DEPTH8                = 2941,
  CG_DEPTH9                = 2942,
  CG_DEPTH10               = 2943,
  CG_DEPTH11               = 2944,
  CG_DEPTH12               = 2945,
  CG_DEPTH13               = 2946,
  CG_DEPTH14               = 2947,
  CG_DEPTH15               = 2948,
  CG_SAMPLE0               = 2949,
  CG_SAMPLE1               = 2950,
  CG_SAMPLE2               = 2951,
  CG_SAMPLE3               = 2952,
  CG_SAMPLE4               = 2953,
  CG_SAMPLE5               = 2954,
  CG_SAMPLE6               = 2955,
  CG_SAMPLE7               = 2956,
  CG_SAMPLE8               = 2957,
  CG_SAMPLE9               = 2958,
  CG_SAMPLE10              = 2959,
  CG_SAMPLE11              = 2960,
  CG_SAMPLE12              = 2961,
  CG_SAMPLE13              = 2962,
  CG_SAMPLE14              = 2963,
  CG_SAMPLE15              = 2964,
  CG_BLENDWEIGHT0          = 3028,
  CG_BLENDWEIGHT1          = 3029,
  CG_BLENDWEIGHT2          = 3030,
  CG_BLENDWEIGHT3          = 3031,
  CG_BLENDWEIGHT4          = 3032,
  CG_BLENDWEIGHT5          = 3033,
  CG_BLENDWEIGHT6          = 3034,
  CG_BLENDWEIGHT7          = 3035,
  CG_BLENDWEIGHT8          = 3036,
  CG_BLENDWEIGHT9          = 3037,
  CG_BLENDWEIGHT10         = 3038,
  CG_BLENDWEIGHT11         = 3039,
  CG_BLENDWEIGHT12         = 3040,
  CG_BLENDWEIGHT13         = 3041,
  CG_BLENDWEIGHT14         = 3042,
  CG_BLENDWEIGHT15         = 3043,
  CG_NORMAL0               = 3092,
  CG_NORMAL1               = 3093,
  CG_NORMAL2               = 3094,
  CG_NORMAL3               = 3095,
  CG_NORMAL4               = 3096,
  CG_NORMAL5               = 3097,
  CG_NORMAL6               = 3098,
  CG_NORMAL7               = 3099,
  CG_NORMAL8               = 3100,
  CG_NORMAL9               = 3101,
  CG_NORMAL10              = 3102,
  CG_NORMAL11              = 3103,
  CG_NORMAL12              = 3104,
  CG_NORMAL13              = 3105,
  CG_NORMAL14              = 3106,
  CG_NORMAL15              = 3107,
  CG_FOGCOORD              = 3156,
  CG_TEXCOORD0             = 3220,
  CG_TEXCOORD1             = 3221,
  CG_TEXCOORD2             = 3222,
  CG_TEXCOORD3             = 3223,
  CG_TEXCOORD4             = 3224,
  CG_TEXCOORD5             = 3225,
  CG_TEXCOORD6             = 3226,
  CG_TEXCOORD7             = 3227,
  CG_TEXCOORD8             = 3228,
  CG_TEXCOORD9             = 3229,
  CG_TEXCOORD10            = 3230,
  CG_TEXCOORD11            = 3231,
  CG_TEXCOORD12            = 3232,
  CG_TEXCOORD13            = 3233,
  CG_TEXCOORD14            = 3234,
  CG_TEXCOORD15            = 3235,
  CG_COMBINER_CONST0       = 3284,
  CG_COMBINER_CONST1       = 3285,
  CG_COMBINER_STAGE_CONST0 = 3286,
  CG_COMBINER_STAGE_CONST1 = 3287,
  CG_OFFSET_TEXTURE_MATRIX = 3288,
  CG_OFFSET_TEXTURE_SCALE  = 3289,
  CG_OFFSET_TEXTURE_BIAS   = 3290,
  CG_CONST_EYE             = 3291,
  CG_COVERAGE              = 3292,
  CG_TESSFACTOR            = 3255,
  CG_GLSL_UNIFORM          = 3300,
  CG_GLSL_ATTRIB           = 3301,
  CG_ENV                   = 3302,
  CG_HLSL_UNIFORM          = 3559,
  CG_HLSL_VARYING          = 3560,
  CG_SAMPLER_RES           = 3561,
  CG_LASTCOL0              = 4400,
  CG_LASTCOL1              = 4401,
  CG_LASTCOL2              = 4402,
  CG_LASTCOL3              = 4403,
  CG_LASTCOL4              = 4404,
  CG_LASTCOL5              = 4405,
  CG_LASTCOL6              = 4406,
  CG_LASTCOL7              = 4407,
  CG_FACE                  = 4410,
  CG_PRIMITIVEID           = 4411,
  CG_INSTANCEID            = 4412,
  CG_SAMPLEID              = 4413,
  CG_VERTEXID              = 4414,
  CG_LAYER                 = 4415,
  CG_SAMPLEMASK            = 4416,
  CG_CONTROLPOINTID        = 4417,
  CG_EDGETESS              = 4418,
  CG_INNERTESS             = 4419,
  CG_SAMPLEPOS             = 4420,
  CG_NUMSAMPLES            = 4421,
  CG_UNDEFINED             = 3256
} CGresource;

typedef enum
{
  CG_PROFILE_UNKNOWN = 6145,
  CG_PROFILE_VP20    = 6146,
  CG_PROFILE_FP20    = 6147,
  CG_PROFILE_VP30    = 6148,
  CG_PROFILE_FP30    = 6149,
  CG_PROFILE_ARBVP1  = 6150,
  CG_PROFILE_FP40    = 6151,
  CG_PROFILE_ARBFP1  = 7000,
  CG_PROFILE_VP40    = 7001,
  CG_PROFILE_GLSLV   = 7007, /* GLSL vertex shader                                       */
  CG_PROFILE_GLSLF   = 7008, /* GLSL fragment shader                                     */
  CG_PROFILE_GLSLG   = 7016, /* GLSL geometry shader                                     */
  CG_PROFILE_GLSLC   = 7009, /* Combined GLSL program                                    */
  CG_PROFILE_GPU_FP  = 7010, /* Deprecated alias for CG_PROFILE_GP4FP                    */
  CG_PROFILE_GPU_VP  = 7011, /* Deprecated alias for CG_PROFILE_GP4VP                    */
  CG_PROFILE_GPU_GP  = 7012, /* Deprecated alias for CG_PROFILE_GP4GP                    */
  CG_PROFILE_GP4FP   = 7010, /* NV_gpu_program4 fragment program                         */
  CG_PROFILE_GP4VP   = 7011, /* NV_gpu_program4 vertex program                           */
  CG_PROFILE_GP4GP   = 7012, /* NV_gpu_program4 geometry program                         */
  CG_PROFILE_GP5FP   = 7017, /* NV_gpu_program5 fragment program                         */
  CG_PROFILE_GP5VP   = 7018, /* NV_gpu_program5 vertex program                           */
  CG_PROFILE_GP5GP   = 7019, /* NV_gpu_program5 geometry program                         */
  CG_PROFILE_GP5TCP  = 7020, /* NV_tessellation_program5 tessellation control program    */
  CG_PROFILE_GP5TEP  = 7021, /* NV_tessellation_program5 tessellation evaluation program */
  CG_PROFILE_VS_1_1  = 6153,
  CG_PROFILE_VS_2_0  = 6154,
  CG_PROFILE_VS_2_X  = 6155,
  CG_PROFILE_VS_2_SW = 6156,
  CG_PROFILE_PS_1_1  = 6159,
  CG_PROFILE_PS_1_2  = 6160,
  CG_PROFILE_PS_1_3  = 6161,
  CG_PROFILE_PS_2_0  = 6162,
  CG_PROFILE_PS_2_X  = 6163,
  CG_PROFILE_PS_2_SW = 6164,
  CG_PROFILE_VS_3_0  = 6157, /* DX9 vertex shader                                        */
  CG_PROFILE_PS_3_0  = 6165, /* DX9 pixel shader                                         */
  CG_PROFILE_HLSLV   = 6158, /* DX9 HLSL vertex shader                                   */
  CG_PROFILE_HLSLF   = 6166, /* DX9 HLSL fragment shader                                 */
  CG_PROFILE_VS_4_0  = 6167, /* DX10 vertex shader                                       */
  CG_PROFILE_PS_4_0  = 6168, /* DX10 pixel shader                                        */
  CG_PROFILE_GS_4_0  = 6169, /* DX10 geometry shader                                     */
  CG_PROFILE_VS_5_0  = 6170, /* DX11 vertex shader                                       */
  CG_PROFILE_PS_5_0  = 6171, /* DX11 pixel shader                                        */
  CG_PROFILE_GS_5_0  = 6172, /* DX11 geometry shader                                     */
  CG_PROFILE_HS_5_0  = 6173, /* DX11 hull shader (tessellation control)                  */
  CG_PROFILE_DS_5_0  = 6174, /* DX11 domain shader (tessellation evaluation)             */
  CG_PROFILE_GENERIC = 7002
} CGprofile;

typedef enum
{
  CG_NO_ERROR                               = 0,
  CG_COMPILER_ERROR                         = 1,
  CG_INVALID_PARAMETER_ERROR                = 2,
  CG_INVALID_PROFILE_ERROR                  = 3,
  CG_PROGRAM_LOAD_ERROR                     = 4,
  CG_PROGRAM_BIND_ERROR                     = 5,
  CG_PROGRAM_NOT_LOADED_ERROR               = 6,
  CG_UNSUPPORTED_GL_EXTENSION_ERROR         = 7,
  CG_INVALID_VALUE_TYPE_ERROR               = 8,
  CG_NOT_MATRIX_PARAM_ERROR                 = 9,
  CG_INVALID_ENUMERANT_ERROR                = 10,
  CG_NOT_4x4_MATRIX_ERROR                   = 11,
  CG_FILE_READ_ERROR                        = 12,
  CG_FILE_WRITE_ERROR                       = 13,
  CG_NVPARSE_ERROR                          = 14,
  CG_MEMORY_ALLOC_ERROR                     = 15,
  CG_INVALID_CONTEXT_HANDLE_ERROR           = 16,
  CG_INVALID_PROGRAM_HANDLE_ERROR           = 17,
  CG_INVALID_PARAM_HANDLE_ERROR             = 18,
  CG_UNKNOWN_PROFILE_ERROR                  = 19,
  CG_VAR_ARG_ERROR                          = 20,
  CG_INVALID_DIMENSION_ERROR                = 21,
  CG_ARRAY_PARAM_ERROR                      = 22,
  CG_OUT_OF_ARRAY_BOUNDS_ERROR              = 23,
  CG_CONFLICTING_TYPES_ERROR                = 24,
  CG_CONFLICTING_PARAMETER_TYPES_ERROR      = 25,
  CG_PARAMETER_IS_NOT_SHARED_ERROR          = 26,
  CG_INVALID_PARAMETER_VARIABILITY_ERROR    = 27,
  CG_CANNOT_DESTROY_PARAMETER_ERROR         = 28,
  CG_NOT_ROOT_PARAMETER_ERROR               = 29,
  CG_PARAMETERS_DO_NOT_MATCH_ERROR          = 30,
  CG_IS_NOT_PROGRAM_PARAMETER_ERROR         = 31,
  CG_INVALID_PARAMETER_TYPE_ERROR           = 32,
  CG_PARAMETER_IS_NOT_RESIZABLE_ARRAY_ERROR = 33,
  CG_INVALID_SIZE_ERROR                     = 34,
  CG_BIND_CREATES_CYCLE_ERROR               = 35,
  CG_ARRAY_TYPES_DO_NOT_MATCH_ERROR         = 36,
  CG_ARRAY_DIMENSIONS_DO_NOT_MATCH_ERROR    = 37,
  CG_ARRAY_HAS_WRONG_DIMENSION_ERROR        = 38,
  CG_TYPE_IS_NOT_DEFINED_IN_PROGRAM_ERROR   = 39,
  CG_INVALID_EFFECT_HANDLE_ERROR            = 40,
  CG_INVALID_STATE_HANDLE_ERROR             = 41,
  CG_INVALID_STATE_ASSIGNMENT_HANDLE_ERROR  = 42,
  CG_INVALID_PASS_HANDLE_ERROR              = 43,
  CG_INVALID_ANNOTATION_HANDLE_ERROR        = 44,
  CG_INVALID_TECHNIQUE_HANDLE_ERROR         = 45,
  CG_INVALID_PARAMETER_HANDLE_ERROR         = 46,
  CG_STATE_ASSIGNMENT_TYPE_MISMATCH_ERROR   = 47,
  CG_INVALID_FUNCTION_HANDLE_ERROR          = 48,
  CG_INVALID_TECHNIQUE_ERROR                = 49,
  CG_INVALID_POINTER_ERROR                  = 50,
  CG_NOT_ENOUGH_DATA_ERROR                  = 51,
  CG_NON_NUMERIC_PARAMETER_ERROR            = 52,
  CG_ARRAY_SIZE_MISMATCH_ERROR              = 53,
  CG_CANNOT_SET_NON_UNIFORM_PARAMETER_ERROR = 54,
  CG_DUPLICATE_NAME_ERROR                   = 55,
  CG_INVALID_OBJ_HANDLE_ERROR               = 56,
  CG_INVALID_BUFFER_HANDLE_ERROR            = 57,
  CG_BUFFER_INDEX_OUT_OF_RANGE_ERROR        = 58,
  CG_BUFFER_ALREADY_MAPPED_ERROR            = 59,
  CG_BUFFER_UPDATE_NOT_ALLOWED_ERROR        = 60,
  CG_GLSLG_UNCOMBINED_LOAD_ERROR            = 61
} CGerror;

typedef enum
{
  CG_UNKNOWN                            = 4096,
  CG_IN                                 = 4097,
  CG_OUT                                = 4098,
  CG_INOUT                              = 4099,
  CG_MIXED                              = 4100,
  CG_VARYING                            = 4101,
  CG_UNIFORM                            = 4102,
  CG_CONSTANT                           = 4103,
  CG_PROGRAM_SOURCE                     = 4104, /* cgGetProgramString                       */
  CG_PROGRAM_ENTRY                      = 4105, /* cgGetProgramString                       */
  CG_COMPILED_PROGRAM                   = 4106, /* cgGetProgramString                       */
  CG_PROGRAM_PROFILE                    = 4107, /* cgGetProgramString                       */
  CG_GLOBAL                             = 4108,
  CG_PROGRAM                            = 4109,
  CG_DEFAULT                            = 4110,
  CG_ERROR                              = 4111,
  CG_SOURCE                             = 4112,
  CG_OBJECT                             = 4113,
  CG_COMPILE_MANUAL                     = 4114,
  CG_COMPILE_IMMEDIATE                  = 4115,
  CG_COMPILE_LAZY                       = 4116,
  CG_CURRENT                            = 4117,
  CG_LITERAL                            = 4118,
  CG_VERSION                            = 4119, /* cgGetString                              */
  CG_ROW_MAJOR                          = 4120,
  CG_COLUMN_MAJOR                       = 4121,
  CG_FRAGMENT                           = 4122, /* cgGetProgramInput and cgGetProgramOutput */
  CG_VERTEX                             = 4123, /* cgGetProgramInput and cgGetProgramOutput */
  CG_POINT                              = 4124, /* Geometry program cgGetProgramInput       */
  CG_LINE                               = 4125, /* Geometry program cgGetProgramInput       */
  CG_LINE_ADJ                           = 4126, /* Geometry program cgGetProgramInput       */
  CG_TRIANGLE                           = 4127, /* Geometry program cgGetProgramInput       */
  CG_TRIANGLE_ADJ                       = 4128, /* Geometry program cgGetProgramInput       */
  CG_POINT_OUT                          = 4129, /* Geometry program cgGetProgramOutput      */
  CG_LINE_OUT                           = 4130, /* Geometry program cgGetProgramOutput      */
  CG_TRIANGLE_OUT                       = 4131, /* Geometry program cgGetProgramOutput      */
  CG_IMMEDIATE_PARAMETER_SETTING        = 4132,
  CG_DEFERRED_PARAMETER_SETTING         = 4133,
  CG_NO_LOCKS_POLICY                    = 4134,
  CG_THREAD_SAFE_POLICY                 = 4135,
  CG_FORCE_UPPER_CASE_POLICY            = 4136,
  CG_UNCHANGED_CASE_POLICY              = 4137,
  CG_IS_OPENGL_PROFILE                  = 4138,
  CG_IS_DIRECT3D_PROFILE                = 4139,
  CG_IS_DIRECT3D_8_PROFILE              = 4140,
  CG_IS_DIRECT3D_9_PROFILE              = 4141,
  CG_IS_DIRECT3D_10_PROFILE             = 4142,
  CG_IS_VERTEX_PROFILE                  = 4143,
  CG_IS_FRAGMENT_PROFILE                = 4144,
  CG_IS_GEOMETRY_PROFILE                = 4145,
  CG_IS_TRANSLATION_PROFILE             = 4146,
  CG_IS_HLSL_PROFILE                    = 4147,
  CG_IS_GLSL_PROFILE                    = 4148,
  CG_IS_TESSELLATION_CONTROL_PROFILE    = 4149,
  CG_IS_TESSELLATION_EVALUATION_PROFILE = 4150,
  CG_PATCH                              = 4152, /* cgGetProgramInput and cgGetProgramOutput */
  CG_IS_DIRECT3D_11_PROFILE             = 4153
} CGenum;

typedef enum
{
  CG_PARAMETERCLASS_UNKNOWN = 0,
  CG_PARAMETERCLASS_SCALAR  = 1,
  CG_PARAMETERCLASS_VECTOR  = 2,
  CG_PARAMETERCLASS_MATRIX  = 3,
  CG_PARAMETERCLASS_STRUCT  = 4,
  CG_PARAMETERCLASS_ARRAY   = 5,
  CG_PARAMETERCLASS_SAMPLER = 6,
  CG_PARAMETERCLASS_OBJECT  = 7,
  CG_PARAMETERCLASS_BUFFER  = 8
} CGparameterclass;

typedef enum
{
  CG_UNKNOWN_DOMAIN                 = 0,
  CG_FIRST_DOMAIN                   = 1,
  CG_VERTEX_DOMAIN                  = 1,
  CG_FRAGMENT_DOMAIN                = 2,
  CG_GEOMETRY_DOMAIN                = 3,
  CG_TESSELLATION_CONTROL_DOMAIN    = 4,
  CG_TESSELLATION_EVALUATION_DOMAIN = 5
} CGdomain;

typedef enum
{
  CG_MAP_READ               = 0,
  CG_MAP_WRITE              = 1,
  CG_MAP_READ_WRITE         = 2,
  CG_MAP_WRITE_DISCARD      = 3,
  CG_MAP_WRITE_NO_OVERWRITE = 4
} CGbufferaccess;

typedef enum
{
  CG_BUFFER_USAGE_STREAM_DRAW  = 0,
  CG_BUFFER_USAGE_STREAM_READ  = 1,
  CG_BUFFER_USAGE_STREAM_COPY  = 2,
  CG_BUFFER_USAGE_STATIC_DRAW  = 3,
  CG_BUFFER_USAGE_STATIC_READ  = 4,
  CG_BUFFER_USAGE_STATIC_COPY  = 5,
  CG_BUFFER_USAGE_DYNAMIC_DRAW = 6,
  CG_BUFFER_USAGE_DYNAMIC_READ = 7,
  CG_BUFFER_USAGE_DYNAMIC_COPY = 8
} CGbufferusage;

#ifdef __cplusplus
extern "C"
{
#endif

typedef CGbool (CGENTRY * CGstatecallback)(CGstateassignment);
typedef void (CGENTRY * CGerrorCallbackFunc)(void);
typedef void (CGENTRY * CGerrorHandlerFunc)(CGcontext, CGerror, void *);
typedef void (CGENTRY * CGIncludeCallbackFunc)(CGcontext, const char *);

/*************************************************************************/
/*** Functions                                                         ***/
/*************************************************************************/

#ifndef CG_EXPLICIT

CG_API CGenum CGENTRY cgSetLockingPolicy(CGenum lockingPolicy);
CG_API CGenum CGENTRY cgGetLockingPolicy(void);
CG_API CGenum CGENTRY cgSetSemanticCasePolicy(CGenum casePolicy);
CG_API CGenum CGENTRY cgGetSemanticCasePolicy(void);
CG_API CGcontext CGENTRY cgCreateContext(void);
CG_API void CGENTRY cgSetContextBehavior(CGcontext context, CGbehavior behavior);
CG_API CGbehavior CGENTRY cgGetContextBehavior(CGcontext context);
CG_API const char * CGENTRY cgGetBehaviorString(CGbehavior behavior);
CG_API CGbehavior CGENTRY cgGetBehavior(const char *behavior_string);
CG_API void CGENTRY cgDestroyContext(CGcontext context);
CG_API CGbool CGENTRY cgIsContext(CGcontext context);
CG_API const char * CGENTRY cgGetLastListing(CGcontext context);
CG_API void CGENTRY cgSetLastListing(CGhandle handle, const char *listing);
CG_API void CGENTRY cgSetAutoCompile(CGcontext context, CGenum autoCompileMode);
CG_API CGenum CGENTRY cgGetAutoCompile(CGcontext context);
CG_API void CGENTRY cgSetParameterSettingMode(CGcontext context, CGenum parameterSettingMode);
CG_API CGenum CGENTRY cgGetParameterSettingMode(CGcontext context);
CG_API void CGENTRY cgSetCompilerIncludeString(CGcontext context, const char *name, const char *source);
CG_API void CGENTRY cgSetCompilerIncludeFile(CGcontext context, const char *name, const char *filename);
CG_API void CGENTRY cgSetCompilerIncludeCallback(CGcontext context, CGIncludeCallbackFunc func);
CG_API CGIncludeCallbackFunc CGENTRY cgGetCompilerIncludeCallback(CGcontext context);
CG_API CGprogram CGENTRY cgCreateProgram(CGcontext context, CGenum program_type, const char *program, CGprofile profile, const char *entry, const char **args);
CG_API CGprogram CGENTRY cgCreateProgramFromFile(CGcontext context, CGenum program_type, const char *program_file, CGprofile profile, const char *entry, const char **args);
CG_API CGprogram CGENTRY cgCopyProgram(CGprogram program);
CG_API void CGENTRY cgDestroyProgram(CGprogram program);
CG_API CGprogram CGENTRY cgGetFirstProgram(CGcontext context);
CG_API CGprogram CGENTRY cgGetNextProgram(CGprogram current);
CG_API CGcontext CGENTRY cgGetProgramContext(CGprogram program);
CG_API CGbool CGENTRY cgIsProgram(CGprogram program);
CG_API void CGENTRY cgCompileProgram(CGprogram program);
CG_API CGbool CGENTRY cgIsProgramCompiled(CGprogram program);
CG_API const char * CGENTRY cgGetProgramString(CGprogram program, CGenum pname);
CG_API CGprofile CGENTRY cgGetProgramProfile(CGprogram program);
CG_API char const * const * CGENTRY cgGetProgramOptions(CGprogram program);
CG_API void CGENTRY cgSetProgramProfile(CGprogram program, CGprofile profile);
CG_API CGenum CGENTRY cgGetProgramInput(CGprogram program);
CG_API CGenum CGENTRY cgGetProgramOutput(CGprogram program);
CG_API int CGENTRY cgGetProgramOutputVertices(CGprogram program);
CG_API void CGENTRY cgSetProgramOutputVertices(CGprogram program, int vertices);
CG_API void CGENTRY cgSetPassProgramParameters(CGprogram program);
CG_API void CGENTRY cgUpdateProgramParameters(CGprogram program);
CG_API void CGENTRY cgUpdatePassParameters(CGpass pass);
CG_API CGparameter CGENTRY cgCreateParameter(CGcontext context, CGtype type);
CG_API CGparameter CGENTRY cgCreateParameterArray(CGcontext context, CGtype type, int length);
CG_API CGparameter CGENTRY cgCreateParameterMultiDimArray(CGcontext context, CGtype type, int dim, const int *lengths);
CG_API void CGENTRY cgDestroyParameter(CGparameter param);
CG_API void CGENTRY cgConnectParameter(CGparameter from, CGparameter to);
CG_API void CGENTRY cgDisconnectParameter(CGparameter param);
CG_API CGparameter CGENTRY cgGetConnectedParameter(CGparameter param);
CG_API int CGENTRY cgGetNumConnectedToParameters(CGparameter param);
CG_API CGparameter CGENTRY cgGetConnectedToParameter(CGparameter param, int index);
CG_API CGparameter CGENTRY cgGetNamedParameter(CGprogram program, const char *name);
CG_API CGparameter CGENTRY cgGetNamedProgramParameter(CGprogram program, CGenum name_space, const char *name);
CG_API CGparameter CGENTRY cgGetNamedProgramUniformBuffer(CGprogram program, const char *blockName);
CG_API CGparameter CGENTRY cgGetNamedEffectUniformBuffer(CGeffect effect, const char *blockName);
CG_API const char * CGENTRY cgGetUniformBufferBlockName(CGparameter param);
CG_API CGparameter CGENTRY cgGetFirstParameter(CGprogram program, CGenum name_space);
CG_API CGparameter CGENTRY cgGetNextParameter(CGparameter current);
CG_API CGparameter CGENTRY cgGetFirstLeafParameter(CGprogram program, CGenum name_space);
CG_API CGparameter CGENTRY cgGetNextLeafParameter(CGparameter current);
CG_API CGparameter CGENTRY cgGetFirstStructParameter(CGparameter param);
CG_API CGparameter CGENTRY cgGetFirstUniformBufferParameter(CGparameter param);
CG_API CGparameter CGENTRY cgGetNamedStructParameter(CGparameter param, const char *name);
CG_API CGparameter CGENTRY cgGetNamedUniformBufferParameter(CGparameter param, const char *name);
CG_API CGparameter CGENTRY cgGetFirstDependentParameter(CGparameter param);
CG_API CGparameter CGENTRY cgGetArrayParameter(CGparameter aparam, int index);
CG_API int CGENTRY cgGetArrayDimension(CGparameter param);
CG_API CGtype CGENTRY cgGetArrayType(CGparameter param);
CG_API int CGENTRY cgGetArraySize(CGparameter param, int dimension);
CG_API int CGENTRY cgGetArrayTotalSize(CGparameter param);
CG_API void CGENTRY cgSetArraySize(CGparameter param, int size);
CG_API void CGENTRY cgSetMultiDimArraySize(CGparameter param, const int *sizes);
CG_API CGprogram CGENTRY cgGetParameterProgram(CGparameter param);
CG_API CGcontext CGENTRY cgGetParameterContext(CGparameter param);
CG_API CGbool CGENTRY cgIsParameter(CGparameter param);
CG_API const char * CGENTRY cgGetParameterName(CGparameter param);
CG_API CGtype CGENTRY cgGetParameterType(CGparameter param);
CG_API CGtype CGENTRY cgGetParameterBaseType(CGparameter param);
CG_API CGparameterclass CGENTRY cgGetParameterClass(CGparameter param);
CG_API int CGENTRY cgGetParameterRows(CGparameter param);
CG_API int CGENTRY cgGetParameterColumns(CGparameter param);
CG_API CGtype CGENTRY cgGetParameterNamedType(CGparameter param);
CG_API const char * CGENTRY cgGetParameterSemantic(CGparameter param);
CG_API CGresource CGENTRY cgGetParameterResource(CGparameter param);
CG_API CGresource CGENTRY cgGetParameterBaseResource(CGparameter param);
CG_API unsigned long CGENTRY cgGetParameterResourceIndex(CGparameter param);
CG_API CGenum CGENTRY cgGetParameterVariability(CGparameter param);
CG_API CGenum CGENTRY cgGetParameterDirection(CGparameter param);
CG_API CGbool CGENTRY cgIsParameterReferenced(CGparameter param);
CG_API CGbool CGENTRY cgIsParameterUsed(CGparameter param, CGhandle handle);
CG_API const double * CGENTRY cgGetParameterValues(CGparameter param, CGenum value_type, int *nvalues);
CG_API void CGENTRY cgSetParameterValuedr(CGparameter param, int nelements, const double *vals);
CG_API void CGENTRY cgSetParameterValuedc(CGparameter param, int nelements, const double *vals);
CG_API void CGENTRY cgSetParameterValuefr(CGparameter param, int nelements, const float *vals);
CG_API void CGENTRY cgSetParameterValuefc(CGparameter param, int nelements, const float *vals);
CG_API void CGENTRY cgSetParameterValueir(CGparameter param, int nelements, const int *vals);
CG_API void CGENTRY cgSetParameterValueic(CGparameter param, int nelements, const int *vals);
CG_API int CGENTRY cgGetParameterValuedr(CGparameter param, int nelements, double *vals);
CG_API int CGENTRY cgGetParameterValuedc(CGparameter param, int nelements, double *vals);
CG_API int CGENTRY cgGetParameterValuefr(CGparameter param, int nelements, float *vals);
CG_API int CGENTRY cgGetParameterValuefc(CGparameter param, int nelements, float *vals);
CG_API int CGENTRY cgGetParameterValueir(CGparameter param, int nelements, int *vals);
CG_API int CGENTRY cgGetParameterValueic(CGparameter param, int nelements, int *vals);
CG_API int CGENTRY cgGetParameterDefaultValuedr(CGparameter param, int nelements, double *vals);
CG_API int CGENTRY cgGetParameterDefaultValuedc(CGparameter param, int nelements, double *vals);
CG_API int CGENTRY cgGetParameterDefaultValuefr(CGparameter param, int nelements, float *vals);
CG_API int CGENTRY cgGetParameterDefaultValuefc(CGparameter param, int nelements, float *vals);
CG_API int CGENTRY cgGetParameterDefaultValueir(CGparameter param, int nelements, int *vals);
CG_API int CGENTRY cgGetParameterDefaultValueic(CGparameter param, int nelements, int *vals);
CG_API const char * CGENTRY cgGetStringParameterValue(CGparameter param);
CG_API void CGENTRY cgSetStringParameterValue(CGparameter param, const char *str);
CG_API int CGENTRY cgGetParameterOrdinalNumber(CGparameter param);
CG_API CGbool CGENTRY cgIsParameterGlobal(CGparameter param);
CG_API int CGENTRY cgGetParameterIndex(CGparameter param);
CG_API void CGENTRY cgSetParameterVariability(CGparameter param, CGenum vary);
CG_API void CGENTRY cgSetParameterSemantic(CGparameter param, const char *semantic);
CG_API void CGENTRY cgSetParameter1f(CGparameter param, float x);
CG_API void CGENTRY cgSetParameter2f(CGparameter param, float x, float y);
CG_API void CGENTRY cgSetParameter3f(CGparameter param, float x, float y, float z);
CG_API void CGENTRY cgSetParameter4f(CGparameter param, float x, float y, float z, float w);
CG_API void CGENTRY cgSetParameter1d(CGparameter param, double x);
CG_API void CGENTRY cgSetParameter2d(CGparameter param, double x, double y);
CG_API void CGENTRY cgSetParameter3d(CGparameter param, double x, double y, double z);
CG_API void CGENTRY cgSetParameter4d(CGparameter param, double x, double y, double z, double w);
CG_API void CGENTRY cgSetParameter1i(CGparameter param, int x);
CG_API void CGENTRY cgSetParameter2i(CGparameter param, int x, int y);
CG_API void CGENTRY cgSetParameter3i(CGparameter param, int x, int y, int z);
CG_API void CGENTRY cgSetParameter4i(CGparameter param, int x, int y, int z, int w);
CG_API void CGENTRY cgSetParameter1iv(CGparameter param, const int *v);
CG_API void CGENTRY cgSetParameter2iv(CGparameter param, const int *v);
CG_API void CGENTRY cgSetParameter3iv(CGparameter param, const int *v);
CG_API void CGENTRY cgSetParameter4iv(CGparameter param, const int *v);
CG_API void CGENTRY cgSetParameter1fv(CGparameter param, const float *v);
CG_API void CGENTRY cgSetParameter2fv(CGparameter param, const float *v);
CG_API void CGENTRY cgSetParameter3fv(CGparameter param, const float *v);
CG_API void CGENTRY cgSetParameter4fv(CGparameter param, const float *v);
CG_API void CGENTRY cgSetParameter1dv(CGparameter param, const double *v);
CG_API void CGENTRY cgSetParameter2dv(CGparameter param, const double *v);
CG_API void CGENTRY cgSetParameter3dv(CGparameter param, const double *v);
CG_API void CGENTRY cgSetParameter4dv(CGparameter param, const double *v);
CG_API void CGENTRY cgSetMatrixParameterir(CGparameter param, const int *matrix);
CG_API void CGENTRY cgSetMatrixParameterdr(CGparameter param, const double *matrix);
CG_API void CGENTRY cgSetMatrixParameterfr(CGparameter param, const float *matrix);
CG_API void CGENTRY cgSetMatrixParameteric(CGparameter param, const int *matrix);
CG_API void CGENTRY cgSetMatrixParameterdc(CGparameter param, const double *matrix);
CG_API void CGENTRY cgSetMatrixParameterfc(CGparameter param, const float *matrix);
CG_API void CGENTRY cgGetMatrixParameterir(CGparameter param, int *matrix);
CG_API void CGENTRY cgGetMatrixParameterdr(CGparameter param, double *matrix);
CG_API void CGENTRY cgGetMatrixParameterfr(CGparameter param, float *matrix);
CG_API void CGENTRY cgGetMatrixParameteric(CGparameter param, int *matrix);
CG_API void CGENTRY cgGetMatrixParameterdc(CGparameter param, double *matrix);
CG_API void CGENTRY cgGetMatrixParameterfc(CGparameter param, float *matrix);
CG_API CGenum CGENTRY cgGetMatrixParameterOrder(CGparameter param);
CG_API CGparameter CGENTRY cgGetNamedSubParameter(CGparameter param, const char *name);
CG_API const char * CGENTRY cgGetTypeString(CGtype type);
CG_API CGtype CGENTRY cgGetType(const char *type_string);
CG_API CGtype CGENTRY cgGetNamedUserType(CGhandle handle, const char *name);
CG_API int CGENTRY cgGetNumUserTypes(CGhandle handle);
CG_API CGtype CGENTRY cgGetUserType(CGhandle handle, int index);
CG_API int CGENTRY cgGetNumParentTypes(CGtype type);
CG_API CGtype CGENTRY cgGetParentType(CGtype type, int index);
CG_API CGbool CGENTRY cgIsParentType(CGtype parent, CGtype child);
CG_API CGbool CGENTRY cgIsInterfaceType(CGtype type);
CG_API const char * CGENTRY cgGetResourceString(CGresource resource);
CG_API CGresource CGENTRY cgGetResource(const char *resource_string);
CG_API const char * CGENTRY cgGetEnumString(CGenum en);
CG_API CGenum CGENTRY cgGetEnum(const char *enum_string);
CG_API const char * CGENTRY cgGetProfileString(CGprofile profile);
CG_API CGprofile CGENTRY cgGetProfile(const char *profile_string);
CG_API int CGENTRY cgGetNumSupportedProfiles(void);
CG_API CGprofile CGENTRY cgGetSupportedProfile(int index);
CG_API CGbool CGENTRY cgIsProfileSupported(CGprofile profile);
CG_API CGbool CGENTRY cgGetProfileProperty(CGprofile profile, CGenum query);
CG_API const char * CGENTRY cgGetParameterClassString(CGparameterclass pc);
CG_API CGparameterclass CGENTRY cgGetParameterClassEnum(const char *pString);
CG_API const char * CGENTRY cgGetDomainString(CGdomain domain);
CG_API CGdomain CGENTRY cgGetDomain(const char *domain_string);
CG_API CGdomain CGENTRY cgGetProgramDomain(CGprogram program);
CG_API CGerror CGENTRY cgGetError(void);
CG_API CGerror CGENTRY cgGetFirstError(void);
CG_API const char * CGENTRY cgGetErrorString(CGerror error);
CG_API const char * CGENTRY cgGetLastErrorString(CGerror *error);
CG_API void CGENTRY cgSetErrorCallback(CGerrorCallbackFunc func);
CG_API CGerrorCallbackFunc CGENTRY cgGetErrorCallback(void);
CG_API void CGENTRY cgSetErrorHandler(CGerrorHandlerFunc func, void *data);
CG_API CGerrorHandlerFunc CGENTRY cgGetErrorHandler(void **data);
CG_API const char * CGENTRY cgGetString(CGenum sname);
CG_API CGeffect CGENTRY cgCreateEffect(CGcontext context, const char *code, const char **args);
CG_API CGeffect CGENTRY cgCreateEffectFromFile(CGcontext context, const char *filename, const char **args);
CG_API CGeffect CGENTRY cgCopyEffect(CGeffect effect);
CG_API void CGENTRY cgDestroyEffect(CGeffect effect);
CG_API CGcontext CGENTRY cgGetEffectContext(CGeffect effect);
CG_API CGbool CGENTRY cgIsEffect(CGeffect effect);
CG_API CGeffect CGENTRY cgGetFirstEffect(CGcontext context);
CG_API CGeffect CGENTRY cgGetNextEffect(CGeffect effect);
CG_API CGprogram CGENTRY cgCreateProgramFromEffect(CGeffect effect, CGprofile profile, const char *entry, const char **args);
CG_API CGtechnique CGENTRY cgGetFirstTechnique(CGeffect effect);
CG_API CGtechnique CGENTRY cgGetNextTechnique(CGtechnique tech);
CG_API CGtechnique CGENTRY cgGetNamedTechnique(CGeffect effect, const char *name);
CG_API const char * CGENTRY cgGetTechniqueName(CGtechnique tech);
CG_API CGbool CGENTRY cgIsTechnique(CGtechnique tech);
CG_API CGbool CGENTRY cgValidateTechnique(CGtechnique tech);
CG_API CGbool CGENTRY cgIsTechniqueValidated(CGtechnique tech);
CG_API CGeffect CGENTRY cgGetTechniqueEffect(CGtechnique tech);
CG_API CGpass CGENTRY cgGetFirstPass(CGtechnique tech);
CG_API CGpass CGENTRY cgGetNamedPass(CGtechnique tech, const char *name);
CG_API CGpass CGENTRY cgGetNextPass(CGpass pass);
CG_API CGbool CGENTRY cgIsPass(CGpass pass);
CG_API const char * CGENTRY cgGetPassName(CGpass pass);
CG_API CGtechnique CGENTRY cgGetPassTechnique(CGpass pass);
CG_API CGprogram CGENTRY cgGetPassProgram(CGpass pass, CGdomain domain);
CG_API void CGENTRY cgSetPassState(CGpass pass);
CG_API void CGENTRY cgResetPassState(CGpass pass);
CG_API CGstateassignment CGENTRY cgGetFirstStateAssignment(CGpass pass);
CG_API CGstateassignment CGENTRY cgGetNamedStateAssignment(CGpass pass, const char *name);
CG_API CGstateassignment CGENTRY cgGetNextStateAssignment(CGstateassignment sa);
CG_API CGbool CGENTRY cgIsStateAssignment(CGstateassignment sa);
CG_API CGbool CGENTRY cgCallStateSetCallback(CGstateassignment sa);
CG_API CGbool CGENTRY cgCallStateValidateCallback(CGstateassignment sa);
CG_API CGbool CGENTRY cgCallStateResetCallback(CGstateassignment sa);
CG_API CGpass CGENTRY cgGetStateAssignmentPass(CGstateassignment sa);
CG_API CGparameter CGENTRY cgGetSamplerStateAssignmentParameter(CGstateassignment sa);
CG_API const float * CGENTRY cgGetFloatStateAssignmentValues(CGstateassignment sa, int *nvalues);
CG_API const int * CGENTRY cgGetIntStateAssignmentValues(CGstateassignment sa, int *nvalues);
CG_API const CGbool * CGENTRY cgGetBoolStateAssignmentValues(CGstateassignment sa, int *nvalues);
CG_API const char * CGENTRY cgGetStringStateAssignmentValue(CGstateassignment sa);
CG_API CGprogram CGENTRY cgGetProgramStateAssignmentValue(CGstateassignment sa);
CG_API CGparameter CGENTRY cgGetTextureStateAssignmentValue(CGstateassignment sa);
CG_API CGparameter CGENTRY cgGetSamplerStateAssignmentValue(CGstateassignment sa);
CG_API int CGENTRY cgGetStateAssignmentIndex(CGstateassignment sa);
CG_API int CGENTRY cgGetNumDependentStateAssignmentParameters(CGstateassignment sa);
CG_API CGparameter CGENTRY cgGetDependentStateAssignmentParameter(CGstateassignment sa, int index);
CG_API int CGENTRY cgGetNumDependentProgramArrayStateAssignmentParameters(CGstateassignment sa);
CG_API CGparameter CGENTRY cgGetDependentProgramArrayStateAssignmentParameter(CGstateassignment sa, int index);
CG_API CGparameter CGENTRY cgGetConnectedStateAssignmentParameter(CGstateassignment sa);
CG_API CGstate CGENTRY cgGetStateAssignmentState(CGstateassignment sa);
CG_API CGstate CGENTRY cgGetSamplerStateAssignmentState(CGstateassignment sa);
CG_API CGstate CGENTRY cgCreateState(CGcontext context, const char *name, CGtype type);
CG_API CGstate CGENTRY cgCreateArrayState(CGcontext context, const char *name, CGtype type, int nelements);
CG_API void CGENTRY cgSetStateCallbacks(CGstate state, CGstatecallback set, CGstatecallback reset, CGstatecallback validate);
CG_API void CGENTRY cgSetStateLatestProfile(CGstate state, CGprofile profile);
CG_API CGstatecallback CGENTRY cgGetStateSetCallback(CGstate state);
CG_API CGstatecallback CGENTRY cgGetStateResetCallback(CGstate state);
CG_API CGstatecallback CGENTRY cgGetStateValidateCallback(CGstate state);
CG_API CGprofile CGENTRY cgGetStateLatestProfile(CGstate state);
CG_API CGcontext CGENTRY cgGetStateContext(CGstate state);
CG_API CGtype CGENTRY cgGetStateType(CGstate state);
CG_API const char * CGENTRY cgGetStateName(CGstate state);
CG_API CGstate CGENTRY cgGetNamedState(CGcontext context, const char *name);
CG_API CGstate CGENTRY cgGetFirstState(CGcontext context);
CG_API CGstate CGENTRY cgGetNextState(CGstate state);
CG_API CGbool CGENTRY cgIsState(CGstate state);
CG_API void CGENTRY cgAddStateEnumerant(CGstate state, const char *name, int value);
CG_API CGstate CGENTRY cgCreateSamplerState(CGcontext context, const char *name, CGtype type);
CG_API CGstate CGENTRY cgCreateArraySamplerState(CGcontext context, const char *name, CGtype type, int nelements);
CG_API CGstate CGENTRY cgGetNamedSamplerState(CGcontext context, const char *name);
CG_API CGstate CGENTRY cgGetFirstSamplerState(CGcontext context);
CG_API CGstateassignment CGENTRY cgGetFirstSamplerStateAssignment(CGparameter param);
CG_API CGstateassignment CGENTRY cgGetNamedSamplerStateAssignment(CGparameter param, const char *name);
CG_API void CGENTRY cgSetSamplerState(CGparameter param);
CG_API CGparameter CGENTRY cgGetNamedEffectParameter(CGeffect effect, const char *name);
CG_API CGparameter CGENTRY cgGetFirstLeafEffectParameter(CGeffect effect);
CG_API CGparameter CGENTRY cgGetFirstEffectParameter(CGeffect effect);
CG_API CGparameter CGENTRY cgGetEffectParameterBySemantic(CGeffect effect, const char *semantic);
CG_API CGannotation CGENTRY cgGetFirstTechniqueAnnotation(CGtechnique tech);
CG_API CGannotation CGENTRY cgGetFirstPassAnnotation(CGpass pass);
CG_API CGannotation CGENTRY cgGetFirstParameterAnnotation(CGparameter param);
CG_API CGannotation CGENTRY cgGetFirstProgramAnnotation(CGprogram program);
CG_API CGannotation CGENTRY cgGetFirstEffectAnnotation(CGeffect effect);
CG_API CGannotation CGENTRY cgGetNextAnnotation(CGannotation ann);
CG_API CGannotation CGENTRY cgGetNamedTechniqueAnnotation(CGtechnique tech, const char *name);
CG_API CGannotation CGENTRY cgGetNamedPassAnnotation(CGpass pass, const char *name);
CG_API CGannotation CGENTRY cgGetNamedParameterAnnotation(CGparameter param, const char *name);
CG_API CGannotation CGENTRY cgGetNamedProgramAnnotation(CGprogram program, const char *name);
CG_API CGannotation CGENTRY cgGetNamedEffectAnnotation(CGeffect effect, const char *name);
CG_API CGbool CGENTRY cgIsAnnotation(CGannotation ann);
CG_API const char * CGENTRY cgGetAnnotationName(CGannotation ann);
CG_API CGtype CGENTRY cgGetAnnotationType(CGannotation ann);
CG_API const float * CGENTRY cgGetFloatAnnotationValues(CGannotation ann, int *nvalues);
CG_API const int * CGENTRY cgGetIntAnnotationValues(CGannotation ann, int *nvalues);
CG_API const char * CGENTRY cgGetStringAnnotationValue(CGannotation ann);
CG_API const char * const * CGENTRY cgGetStringAnnotationValues(CGannotation ann, int *nvalues);
CG_API const CGbool * CGENTRY cgGetBoolAnnotationValues(CGannotation ann, int *nvalues);
CG_API const int * CGENTRY cgGetBooleanAnnotationValues(CGannotation ann, int *nvalues);
CG_API int CGENTRY cgGetNumDependentAnnotationParameters(CGannotation ann);
CG_API CGparameter CGENTRY cgGetDependentAnnotationParameter(CGannotation ann, int index);
CG_API void CGENTRY cgEvaluateProgram(CGprogram program, float *buf, int ncomps, int nx, int ny, int nz);
CG_API CGbool CGENTRY cgSetEffectName(CGeffect effect, const char *name);
CG_API const char * CGENTRY cgGetEffectName(CGeffect effect);
CG_API CGeffect CGENTRY cgGetNamedEffect(CGcontext context, const char *name);
CG_API CGparameter CGENTRY cgCreateEffectParameter(CGeffect effect, const char *name, CGtype type);
CG_API CGtechnique CGENTRY cgCreateTechnique(CGeffect effect, const char *name);
CG_API CGparameter CGENTRY cgCreateEffectParameterArray(CGeffect effect, const char *name, CGtype type, int length);
CG_API CGparameter CGENTRY cgCreateEffectParameterMultiDimArray(CGeffect effect, const char *name, CGtype type, int dim, const int *lengths);
CG_API CGpass CGENTRY cgCreatePass(CGtechnique tech, const char *name);
CG_API CGstateassignment CGENTRY cgCreateStateAssignment(CGpass pass, CGstate state);
CG_API CGstateassignment CGENTRY cgCreateStateAssignmentIndex(CGpass pass, CGstate state, int index);
CG_API CGstateassignment CGENTRY cgCreateSamplerStateAssignment(CGparameter param, CGstate state);
CG_API CGbool CGENTRY cgSetFloatStateAssignment(CGstateassignment sa, float value);
CG_API CGbool CGENTRY cgSetIntStateAssignment(CGstateassignment sa, int value);
CG_API CGbool CGENTRY cgSetBoolStateAssignment(CGstateassignment sa, CGbool value);
CG_API CGbool CGENTRY cgSetStringStateAssignment(CGstateassignment sa, const char *value);
CG_API CGbool CGENTRY cgSetProgramStateAssignment(CGstateassignment sa, CGprogram program);
CG_API CGbool CGENTRY cgSetSamplerStateAssignment(CGstateassignment sa, CGparameter param);
CG_API CGbool CGENTRY cgSetTextureStateAssignment(CGstateassignment sa, CGparameter param);
CG_API CGbool CGENTRY cgSetFloatArrayStateAssignment(CGstateassignment sa, const float *vals);
CG_API CGbool CGENTRY cgSetIntArrayStateAssignment(CGstateassignment sa, const int *vals);
CG_API CGbool CGENTRY cgSetBoolArrayStateAssignment(CGstateassignment sa, const CGbool *vals);
CG_API CGannotation CGENTRY cgCreateTechniqueAnnotation(CGtechnique tech, const char *name, CGtype type);
CG_API CGannotation CGENTRY cgCreatePassAnnotation(CGpass pass, const char *name, CGtype type);
CG_API CGannotation CGENTRY cgCreateParameterAnnotation(CGparameter param, const char *name, CGtype type);
CG_API CGannotation CGENTRY cgCreateProgramAnnotation(CGprogram program, const char *name, CGtype type);
CG_API CGannotation CGENTRY cgCreateEffectAnnotation(CGeffect effect, const char *name, CGtype type);
CG_API CGbool CGENTRY cgSetIntAnnotation(CGannotation ann, int value);
CG_API CGbool CGENTRY cgSetFloatAnnotation(CGannotation ann, float value);
CG_API CGbool CGENTRY cgSetBoolAnnotation(CGannotation ann, CGbool value);
CG_API CGbool CGENTRY cgSetStringAnnotation(CGannotation ann, const char *value);
CG_API int CGENTRY cgGetNumStateEnumerants(CGstate state);
CG_API const char * CGENTRY cgGetStateEnumerant(CGstate state, int index, int *value);
CG_API const char * CGENTRY cgGetStateEnumerantName(CGstate state, int value);
CG_API int CGENTRY cgGetStateEnumerantValue(CGstate state, const char *name);
CG_API CGeffect CGENTRY cgGetParameterEffect(CGparameter param);
CG_API CGparameterclass CGENTRY cgGetTypeClass(CGtype type);
CG_API CGtype CGENTRY cgGetTypeBase(CGtype type);
CG_API CGbool CGENTRY cgGetTypeSizes(CGtype type, int *nrows, int *ncols);
CG_API void CGENTRY cgGetMatrixSize(CGtype type, int *nrows, int *ncols);
CG_API int CGENTRY cgGetNumProgramDomains(CGprogram program);
CG_API CGdomain CGENTRY cgGetProfileDomain(CGprofile profile);
CG_API CGprofile CGENTRY cgGetProfileSibling(CGprofile profile, CGdomain domain);
CG_API CGprogram CGENTRY cgCombinePrograms(int n, const CGprogram *exeList);
CG_API CGprogram CGENTRY cgCombinePrograms2(const CGprogram exe1, const CGprogram exe2);
CG_API CGprogram CGENTRY cgCombinePrograms3(const CGprogram exe1, const CGprogram exe2, const CGprogram exe3);
CG_API CGprogram CGENTRY cgCombinePrograms4(const CGprogram exe1, const CGprogram exe2, const CGprogram exe3, const CGprogram exe4);
CG_API CGprogram CGENTRY cgCombinePrograms5(const CGprogram exe1, const CGprogram exe2, const CGprogram exe3, const CGprogram exe4, const CGprogram exe5);
CG_API CGprofile CGENTRY cgGetProgramDomainProfile(CGprogram program, int index);
CG_API CGprogram CGENTRY cgGetProgramDomainProgram(CGprogram program, int index);
CG_API CGobj CGENTRY cgCreateObj(CGcontext context, CGenum program_type, const char *source, CGprofile profile, const char **args);
CG_API CGobj CGENTRY cgCreateObjFromFile(CGcontext context, CGenum program_type, const char *source_file, CGprofile profile, const char **args);
CG_API void CGENTRY cgDestroyObj(CGobj obj);
CG_API long CGENTRY cgGetParameterResourceSize(CGparameter param);
CG_API CGtype CGENTRY cgGetParameterResourceType(CGparameter param);
CG_API const char * CGENTRY cgGetParameterResourceName(CGparameter param);
CG_API int CGENTRY cgGetParameterBufferIndex(CGparameter param);
CG_API int CGENTRY cgGetParameterBufferOffset(CGparameter param);
CG_API CGbuffer CGENTRY cgCreateBuffer(CGcontext context, int size, const void *data, CGbufferusage bufferUsage);
CG_API CGbool CGENTRY cgIsBuffer(CGbuffer buffer);
CG_API void CGENTRY cgSetBufferData(CGbuffer buffer, int size, const void *data);
CG_API void CGENTRY cgSetBufferSubData(CGbuffer buffer, int offset, int size, const void *data);
CG_API void CGENTRY cgSetProgramBuffer(CGprogram program, int bufferIndex, CGbuffer buffer);
CG_API void CGENTRY cgSetUniformBufferParameter(CGparameter param, CGbuffer buffer);
CG_API void * CGENTRY cgMapBuffer(CGbuffer buffer, CGbufferaccess access);
CG_API void CGENTRY cgUnmapBuffer(CGbuffer buffer);
CG_API void CGENTRY cgDestroyBuffer(CGbuffer buffer);
CG_API CGbuffer CGENTRY cgGetProgramBuffer(CGprogram program, int bufferIndex);
CG_API CGbuffer CGENTRY cgGetUniformBufferParameter(CGparameter param);
CG_API int CGENTRY cgGetBufferSize(CGbuffer buffer);
CG_API int CGENTRY cgGetProgramBufferMaxSize(CGprofile profile);
CG_API int CGENTRY cgGetProgramBufferMaxIndex(CGprofile profile);
CG_API CGbuffer CGENTRY cgGetEffectParameterBuffer(CGparameter param);
CG_API void CGENTRY cgSetEffectParameterBuffer(CGparameter param, CGbuffer buffer);

#endif

#ifdef __cplusplus
}
#endif

#ifdef CG_APIENTRY_DEFINED
# undef CG_APIENTRY_DEFINED
# undef APIENTRY
#endif

#ifdef CG_WINGDIAPI_DEFINED
# undef CG_WINGDIAPI_DEFINED
# undef WINGDIAPI
#endif

#endif
