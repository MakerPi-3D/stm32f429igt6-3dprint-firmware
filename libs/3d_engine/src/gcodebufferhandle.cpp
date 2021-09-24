#include "gcodebufferhandle.h"
#include "threed_engine.h"
#include "sysconfig_data.h"
#include "user_ccm.h"
#include "user_common.h"
#ifdef __cplusplus
extern "C" {
#endif

static const int gcode_buffer_size = 96;                      /*!< gcode�����С */

// GcodeBufHandle variables
static uint32_t ForceChksum_RetryCnt;                         /*!< ��$ǿ��У�����Լ��� */
static uint32_t ForceChksum_ErrorCnt;                         /*!< ��$ǿ��У�������� */
static uint32_t check_unknown_cmd_retry_count;                /*!< δָ֪��У�����Լ��� */
static uint32_t check_unknown_cmd_error_count;                /*!< δָ֪��У�������� */
static uint32_t checksum_error_counts;                        /*!< ��ָ֪��У�����Լ��� */
static uint32_t checksum_retry_counts;                        /*!< ��ָ֪��У�������� */

static volatile uint32_t cmd_buf_pos;                         /*!< ��ǰָ��λ�� */
static volatile uint32_t leave_file_size;                     /*!< ʣ���ļ���С */

static bool is_comment_mode;                                  /*!< �Ƿ�Ϊע�� */
static uint8_t command_buffer_count;                          /*!< ָ��������� */
static bool is_force_verify;                                  /*!< �Ƿ���ǿ��У�� */

void setForceVerify(bool value)
{
  gcodeBufHandle.setForceVerify(value);
}

void resetCmdBuf(void)
{
  gcodeBufHandle.reset();
}

void set_leave_file_size(uint32_t FileSize)
{
  gcodeBufHandle.setLeaveFileSize(FileSize);
}

uint8_t GetGcodeFromBuf(const char sd_char, const uint32_t file_position, bool isColorMix, volatile uint32_t &file_size, unsigned int &sd_buffersize)
{
  return gcodeBufHandle.getFromGcode(sd_char, file_position, isColorMix, file_size, sd_buffersize);
}


#ifdef __cplusplus
} //extern "C" {
#endif

// ParseGcodeBufHandle variables
static char GcodeBuffer[gcode_buffer_size] = {0};             /*!< gcode���� */
static char *strchr_pointer;                                  /*!< just a pointer to find chars in the cmd string like X, Y, Z, E, etc */
ParseGcodeBufHandle parseGcodeBufHandle;

// GcodeBufHandle variables
GcodeBufHandle gcodeBufHandle;

/**
 *
 */
ParseGcodeBufHandle::ParseGcodeBufHandle()
{
  strchr_pointer = NULL;
  memset(GcodeBuffer, 0, gcode_buffer_size);
}

/**
 * [ParseGcodeBufHandle::codeValue description]
 * @return [description]
 */
float ParseGcodeBufHandle::codeValue()
{
  return strtof(strchr_pointer + 1, NULL);
}

/**
 * [ParseGcodeBufHandle::codeValueLong description]
 * @return [description]
 */
long ParseGcodeBufHandle::codeValueLong()
{
  return (strtol(strchr_pointer + 1, NULL, 10));
}

bool ParseGcodeBufHandle::firstCodeSeen(const char code)
{
  if (GcodeBuffer[0] == code)
  {
    strchr_pointer = strchr(GcodeBuffer, code);
    return true;
  }
  else
  {
    return false;
  }
}

bool ParseGcodeBufHandle::codeSeen(const char code)
{
  strchr_pointer = strchr(GcodeBuffer, code);

  if (strchr_pointer != NULL)
    return true;
  else
    return false;
}

bool ParseGcodeBufHandle::codeSeenStr(const char *code)
{
  char *strstr_pointer = strstr(GcodeBuffer, code);

  if (strstr_pointer != NULL)
    return true;
  else
    return false;
}

void ParseGcodeBufHandle::setGcodeBuffer(const char *gcode_buffer)
{
  memset(GcodeBuffer, 0, sizeof(char) * gcode_buffer_size); // ����ַ���
  size_t buf_len = (strlen(gcode_buffer) <= gcode_buffer_size ? strlen(gcode_buffer) : gcode_buffer_size); // �ַ�������
  strncpy(GcodeBuffer, gcode_buffer, buf_len); // �����ַ���
  //  USER_EchoLog("%s\r\n", GcodeBuffer);
  // �޸�������ӡ��һ��û�仯��cura���ɵ�gcode���˿ո񣬵���У׼�쳣������ָ��
  // ����޸��󣬿�ɾ�����߼�
  char *pStr = strstr(GcodeBuffer, " E ");

  if (NULL != pStr)
  {
    strcpy(pStr + 2, pStr + 3);
    buf_len--;
  }
}

char *ParseGcodeBufHandle::getGcodeBuf()
{
  return GcodeBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
GcodeBufHandle::GcodeBufHandle()
{
  reset();
}

/**
 * [GcodeBufHandle::reset description]
 */
void GcodeBufHandle::reset(void)
{
  ForceChksum_RetryCnt = 0;
  ForceChksum_ErrorCnt = 0;
  check_unknown_cmd_retry_count = 0;
  check_unknown_cmd_error_count = 0;
  checksum_error_counts = 0;
  checksum_retry_counts = 0;
  cmd_buf_pos = 0;
  leave_file_size = 0;
  is_comment_mode = false;
  memset(ccm_param::command_buffer, 0, gcode_buffer_size);
  command_buffer_count = 0;
  is_force_verify = false;
}
/*20170807�����޸�*/
void GcodeBufHandle::setLeaveFileSize(uint32_t FileSize)
{
  leave_file_size = FileSize;
}

void GcodeBufHandle::setForceVerify(bool isForceVerify)
{
  is_force_verify = isForceVerify;
}

static void Judge_Color(void)
{
  if (t_sys.enable_color_buf == 1)
  {
    if (strstr((char *)ccm_param::command_buffer, "G428 R6"))
    {
      t_sys.enable_color_buf = 0;
    }
  }
  else if (t_sys.enable_color_buf == 0)
  {
    if (strstr((char *)ccm_param::command_buffer, "G428 R7"))
    {
      t_sys.enable_color_buf = 1;
    }
  }
}



bool GcodeBufHandle::getFromGcode(const char sd_char, const uint32_t file_position, bool isColorMix, volatile uint32_t &file_size, unsigned int &sd_buffersize)
{
  if ((sd_char == '\r')  || (sd_char == '\n'))
  {
    ccm_param::command_buffer[command_buffer_count] = 0; // ����ַ���������
    // �޸�������ӡ��һ��û�仯��cura���ɵ�gcode���˿ո񣬵���У׼�쳣������ָ��
    // ����޸��󣬿�ɾ�����߼�
    char *pStr = strstr(ccm_param::command_buffer, " E ");

    if (NULL != pStr)
    {
      strcpy(pStr + 2, pStr + 3);
      command_buffer_count--;
    }

    if (ccm_param::command_buffer[0] == ';')
    {
      char *layer_p = strstr(&ccm_param::command_buffer[0], ";LAYER:");

      if (layer_p != NULL)
      {
        ccm_param::current_layer = (strtol(layer_p + 7, NULL, 10));
      }

      layer_p = strstr(&ccm_param::command_buffer[0], ";LAYER_COUNT:");

      if (layer_p != NULL)
      {
        ccm_param::layer_count = (strtol(layer_p + 13, NULL, 10));
      }

      memset(ccm_param::command_buffer, 0, gcode_buffer_size); // ���������Ѷ��ַ�
      command_buffer_count = 0;
      is_comment_mode = false;
      return 0;
    }

    if (!command_buffer_count)                // ���л�ֱ�ӷֺŵ�ע���У�����
    {
      is_comment_mode = false;
      return 0;
    }
    else if (is_comment_mode)       //�д��ֺ�ע�͵�Gcode������
    {
      Judge_Color();
      updateCmdBuf(&ccm_param::command_buffer[0], isColorMix);                  // ���µ�ǰbuffer
      user_send_file_cmd(ccm_param::command_buffer, file_position, ccm_param::layer_count, ccm_param::current_layer);
      //      (void)snprintf(&ccm_param::command_buffer[command_buffer_count], 20, " P%u", file_position); // �����ļ�λ��
      //      user_send_str(GCODE_TYPE_FILE, ccm_param::command_buffer);
      command_buffer_count = 0; //clear buffer
      is_comment_mode = false;
      return 1;
    }
    else       //�����ֺ�ע�͵�Gcode������
    {
      //��ǰ������У��
      if (verifyCmdBuf(&ccm_param::command_buffer[0], is_force_verify, isColorMix, file_size, sd_buffersize) == false)
      {
        memset(ccm_param::command_buffer, 0, gcode_buffer_size); // ���������Ѷ��ַ�
        command_buffer_count = 0;
        is_comment_mode = false;
        return 0;
      }

      //    USER_DbgLog("file_position in getCmdBufFromGcode=%u",file_position);
      user_send_file_cmd(ccm_param::command_buffer, file_position, ccm_param::layer_count, ccm_param::current_layer);
      //      (void)snprintf(&ccm_param::command_buffer[command_buffer_count], 20, " P%u", file_position); //�����ļ�λ��
      //      user_send_str(GCODE_TYPE_FILE, ccm_param::command_buffer);
      command_buffer_count = 0; //clear buffer
      is_comment_mode = false;
      cmd_buf_pos = sd_buffersize;
      leave_file_size = file_size;
      return 1;
    }
  }
  else
  {
    if (command_buffer_count == 0) //������;��ͷ�ַ�����ʶ������
    {
      if (sd_char == '(')
        is_comment_mode = true;  //��⵽�ֺ�}
    }
    else
    {
      if (sd_char == ';' || sd_char == '(')
        is_comment_mode = true;  //��⵽�ֺ�
    }

    if (!is_comment_mode)   //������ݣ����˵��ֺź������
    {
      if (command_buffer_count >= gcode_buffer_size)
        ccm_param::command_buffer[gcode_buffer_size - 1] = '\r';  //�������������ض����ݣ��Է����
      else
        ccm_param::command_buffer[command_buffer_count++] = sd_char;
    }
  }

  return 0;
}

void GcodeBufHandle::updateCmdBuf(char *curr_buf, bool isDecrypt)
{
  if (!t_sys.enable_color_buf)
  {
    return;
  }

  char decryptBuf_tmp[gcode_buffer_size] = {0};
  int pos = 0;

  while (curr_buf[pos] != '$')
  {
    // ɨ�赽У���ַ�$���˳�
    if (pos + 1 < gcode_buffer_size && curr_buf[pos] == ' ' && curr_buf[pos + 1] == '$')
      break;

    // ���뵱ǰ�ַ�
    decryptBuf_tmp[pos] = isDecrypt ? user_decryption_code(curr_buf[pos], 20, pos) : curr_buf[pos];

    // �����������ֵ���˳�
    if (++pos == gcode_buffer_size)
      break;
  }

  // ����ַ���������
  if (pos < gcode_buffer_size)
    decryptBuf_tmp[pos] = 0;

  command_buffer_count = pos;
  // ��cmdbuffer_temp���Ƶ�curr_buf
  memcpy(curr_buf, decryptBuf_tmp, gcode_buffer_size);
}

//��cmdbufferУ����
uint8_t GcodeBufHandle::getCheckSum(const char *buf)
{
  // ��ʼ��У��ֵ
  uint8_t chksum = buf[0];

  for (int i = 1; i < gcode_buffer_size; i++)
  {
    // ɨ�赽У���ַ�$���˳�
    if (i + 1 < gcode_buffer_size && buf[i] == ' ' && buf[i + 1] == '$')
      break;

    chksum ^= buf[i];
  }

  return chksum;
}
void GcodeBufHandle::verifyCmdBufCount(int loopCnt, int &currCnt, uint32_t &retryCnt, uint32_t &ErrorCnt, volatile uint32_t &file_size, unsigned int &sd_buffersize)
{
  // У�����С��loopCnt�����¶�ȡָ��
  if (currCnt < loopCnt)
  {
    sd_buffersize = cmd_buf_pos;       // ����pos

    if (leave_file_size) //20170830��ֹfile_size����Ϊ0
      file_size = leave_file_size;

    retryCnt++;                        // �ض�����
    currCnt++;                         // У�������һ
  }
  // У���������loopCnt��ָ�����������ָ��
  else if (currCnt == loopCnt)
  {
    ErrorCnt++;                        // �������
    currCnt = 0;                       // У���������Ϊ0
  }

  command_buffer_count = 0;            // ������ַ�����
}
bool GcodeBufHandle::verifyKnownCmd(bool isKnownCmd, bool isChkSumCorrect, volatile uint32_t &file_size, unsigned int &sd_buffersize)
{
  //known command
  static int unknown_cmd_counts = 0;
  static int chksum_loop_cnt = 0;

  // ��ָ֪��
  if (isKnownCmd)
  {
    if (!isChkSumCorrect)
    {
      verifyCmdBufCount(20, chksum_loop_cnt, checksum_retry_counts, checksum_error_counts, file_size, sd_buffersize);
      return false;//return;
    }
    else
    {
      chksum_loop_cnt = 0;
    }

    unknown_cmd_counts = 0;
  }
  else
  {
    //unknown command
    verifyCmdBufCount(3, unknown_cmd_counts, check_unknown_cmd_retry_count, check_unknown_cmd_error_count, file_size, sd_buffersize);
    return false;
  }

  return true;
}
bool GcodeBufHandle::verifyCmdBuf(char *curr_cmd, bool force_verify, bool isColorMix, volatile uint32_t &file_size, unsigned int &sd_buffersize)
{
  // ����ǿ��У�����
  static int verifyForceCount = 0;
  // ��ȡ��Ԫ����λ��
  char *find_dollar_char = strchr(curr_cmd, '$');

  // M305 S1ʱ������ǿ��ģʽ��ָ��û��У�����$��ֱ��������ָ��
  if (find_dollar_char == NULL && force_verify)
  {
    verifyCmdBufCount(3, verifyForceCount, ForceChksum_RetryCnt, ForceChksum_ErrorCnt, file_size, sd_buffersize);
    return false;
  }
  else
  {
    verifyForceCount = 0; // У���������
  }

  if (find_dollar_char != NULL)
  {
    uint8_t get_buf_chksum = (uint8_t)(strtod(find_dollar_char + 1, NULL));             // ��ȡ�����и���У��ֵ
    uint8_t cal_buf_chksum = getCheckSum(curr_cmd);                                     // ����������У��ֵ
    bool isChkSumCorrect = (get_buf_chksum == cal_buf_chksum || (cal_buf_chksum^' ') == get_buf_chksum) ? true : false;           // �Ƚϲ�����У����
    updateCmdBuf(curr_cmd, isColorMix);                                                 // ���뵽��ǰbuffer
    bool isKnownCmd = (curr_cmd[0] == 'G' || curr_cmd[0] == 'M' || curr_cmd[0] == 'T'); // �Ƿ�Ϊ��ָ֪��
    return verifyKnownCmd(isKnownCmd, isChkSumCorrect, file_size, sd_buffersize);
  }

  return true;
}
extern int16_t target_temperature[];
extern int16_t target_temperature_bak[];
//extern int16_t target_temperature_buf;
void setM109GcodeBuffer(bool mark)
{
  memset(GcodeBuffer, 0, sizeof(GcodeBuffer)); // ����ַ���
  (void)snprintf(GcodeBuffer, sizeof(GcodeBuffer), "M109 T%d S%d", mark, target_temperature_bak[mark]);
}

