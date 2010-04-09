#ifndef __MDFN_ERROR_H
#define __MDFN_ERROR_H

#ifdef __cplusplus
class MDFN_Error
{
 public:

 MDFN_Error(int errno_code_new, const char *format, ...);
 ~MDFN_Error();

 MDFN_Error(const MDFN_Error &ze_error);

 const char *GetErrorMessage(void);
 int GetErrno(void);

 private:

 int errno_code;
 char *error_message;
};
#endif

#endif
