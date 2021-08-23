#ifndef RTPPARAM_H
#define RTPPARAM_H



// It is also recommended that 1/4 of the RTCP bandwidth be dedicated to participants that are sending data
#define RTCP_BANDWIDTH_FRACTION			0.05
#define RTCP_SENDER_BANDWIDTH_FRACTION	0.25

#define RTCP_REPORT_INTERVAL			5000 /* milliseconds RFC3550 p25 */
#define RTCP_REPORT_INTERVAL_MIN		2500 /* milliseconds RFC3550 p25 */

#define RTP_PAYLOAD_MAX_SIZE			(10 * 1024 * 1024)


#endif // RTPPARAM_H
