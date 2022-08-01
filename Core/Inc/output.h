/*
 * output.h
 *
 *  Created on: May 2, 2022
 *      Author: marius
 */

#ifndef APPLICATION_USER_INCLUDE_OUTPUT_H_
#define APPLICATION_USER_INCLUDE_OUTPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void output_string (const char* s);
extern void output (const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_USER_INCLUDE_OUTPUT_H_ */
