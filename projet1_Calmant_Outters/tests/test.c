#include <CUnit/Basic.h>
#include "../src/packet_interface.h"
#include "../src/transport_interface.h"


void test_pkt_new(){
  pkt_t * pkt = pkt_new();

  CU_ASSERT_PTR_NOT_NULL(pkt);

  pkt_del(pkt);
}

void test_set_get(){
  pkt_t * pkt = pkt_new();

  char * payload = "test";
  int len = sizeof(payload);

  pkt_set_type(pkt, PTYPE_DATA);
  pkt_set_window(pkt, 10);
  pkt_set_tr(pkt, 0);
  pkt_set_timestamp(pkt, 0);
  pkt_set_payload(pkt, payload, len);
  pkt_set_seqnum(pkt, 0);
  pkt_set_crc1(pkt, 42);
  pkt_set_crc2(pkt, 42);

  CU_ASSERT_EQUAL(pkt_get_type(pkt), PTYPE_DATA);
  CU_ASSERT_EQUAL(pkt_get_window(pkt), 10);
  CU_ASSERT_EQUAL(pkt_get_tr(pkt), 0);
  CU_ASSERT_EQUAL(pkt_get_timestamp(pkt), 0);
  CU_ASSERT_EQUAL(memcmp(pkt_get_payload(pkt), payload, len), 0);
  CU_ASSERT_EQUAL(pkt_get_seqnum(pkt), 0);
  CU_ASSERT_EQUAL(pkt_get_crc1(pkt), 42);
  CU_ASSERT_EQUAL(pkt_get_crc2(pkt), 42);

  pkt_del(pkt);
}
/*

 struct sockaddr_in6 {
        u_char           sin6_len;      // length of this structure
        u_char           sin6_family;   // AF_INET6
        u_int16m_t       sin6_port;     // Transport layer port #
        u_int32m_t       sin6_flowinfo; // IPv6 flow information
        struct in6_addr  sin6_addr;     // IPv6 address
 };

 struct in6_addr {
        u_int8_t  s6_addr[16];  // IPv6 address
}

*/

void test_real_address(){

  struct sockaddr_in6 localhost;
  struct sockaddr_in6 localhost_1;
  struct sockaddr_in6 rv;

  memset(localhost.sin6_addr.s6_addr, 0, 16);
  memset(localhost_1.sin6_addr.s6_addr, 0, 16);

  localhost_1.sin6_addr.s6_addr[15] = 1;

  localhost.sin6_family =  AF_INET6;
  localhost_1.sin6_family =  AF_INET6;

  real_address("localhost", &rv);
  int cmp = memcmp(rv.sin6_addr.s6_addr, localhost.sin6_addr.s6_addr, 16);

  if( cmp != 0){
    CU_ASSERT_EQUAL(memcmp(rv.sin6_addr.s6_addr, localhost_1.sin6_addr.s6_addr, 16), 0);
  }
  else
    CU_ASSERT_EQUAL(cmp, 0);
}


void test_inc_seqnum(){
  int seqnum = 0 ;
  seqnum = inc_seqnum(seqnum);
  CU_ASSERT_EQUAL(seqnum, 1);
  seqnum = 255;
  seqnum = inc_seqnum(seqnum);
  CU_ASSERT_EQUAL(seqnum, 0);
}

void test_create_socket(){

   struct sockaddr_in6 source;
   real_address("localhost", &source);

   int rv = create_socket(&source, 9858, NULL, 0);

  CU_ASSERT_TRUE(rv > 0);

  close(rv);

}

void test_(){


}

int setup(void)
{
	return 0;
}

int teardown(void)
{
	return 0;
}

int main(void) {
  if(CUE_SUCCESS != CU_initialize_registry())
    return CU_get_error();

  CU_pSuite pSuite = NULL;
  pSuite = CU_add_suite("ma_suite",setup,teardown);
  if(NULL == pSuite)
  {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if((NULL == CU_add_test(pSuite,"Test_pkt_new", test_pkt_new))
    ||(NULL == CU_add_test(pSuite,"Test_set_get", test_set_get))
    ||(NULL == CU_add_test(pSuite,"Test_real_address", test_real_address))
    ||(NULL == CU_add_test(pSuite,"test_create_socket", test_create_socket))
    ||(NULL == CU_add_test(pSuite,"Test_inc_seqnum", test_inc_seqnum))
    )
  {
    CU_cleanup_registry();
    return CU_get_error();
  }
  CU_basic_run_tests();
  CU_basic_show_failures(CU_get_failure_list());

  return 0;
}
