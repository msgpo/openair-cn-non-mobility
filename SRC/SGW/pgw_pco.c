/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file pgw_pco.c
  \brief
  \author Lionel Gauthier
  \company Eurecom
  \email: lionel.gauthier@eurecom.fr
*/
#include <stdint.h>
#include "bstrlib.h"
#include "assertions.h"
#include "log.h"
#include "pgw_pco.h"
#include "common_defs.h"
#include "rfc_1877.h"
#include "rfc_1332.h"
#include "spgw_config.h"
//------------------------------------------------------------------------------
int pgw_pco_push_protocol_or_container_id(protocol_configuration_options_t * const pco, pco_protocol_or_container_id_t * const poc_id)
{
  if (PCO_UNSPEC_MAXIMUM_PROTOCOL_ID_OR_CONTAINER_ID <= pco->num_protocol_or_container_id) {
    return RETURNerror;
  }
  pco->protocol_or_container_ids[pco->num_protocol_or_container_id].id = poc_id->id;
  pco->protocol_or_container_ids[pco->num_protocol_or_container_id].length = poc_id->length;
  pco->protocol_or_container_ids[pco->num_protocol_or_container_id].contents = poc_id->contents;
  poc_id->contents = NULL;
  pco->num_protocol_or_container_id += 1;
  return RETURNok;
}
//------------------------------------------------------------------------------
void pgw__process_pco_request_push_pco(protocol_configuration_options_t * const pco_resp)
{
  // spgw_config.pgw_config.force_push_pco
  AssertFatal(0!=0, "TODO code for pushing PCO to UE");
}
//------------------------------------------------------------------------------
int pgw_process_pco_request_ipcp(protocol_configuration_options_t * const pco_resp, const pco_protocol_or_container_id_t * const poc_id)
{
  uint32_t                                ipcp_dns_prim_ipv4_addr = 0xFFFFFFFF;
  uint32_t                                ipcp_dns_sec_ipv4_addr = 0xFFFFFFFF;
  uint32_t                                ipcp_out_dns_prim_ipv4_addr = 0xFFFFFFFF;
  uint32_t                                ipcp_out_dns_sec_ipv4_addr = 0xFFFFFFFF;
  pco_protocol_or_container_id_t          poc_id_resp = {0};
  size_t                                  ipcp_req_remaining_length = poc_id->length;
  size_t                                  pco_in_index = 0;

  uint8_t                                 ipcp_req_code = 0;
  uint8_t                                 ipcp_req_identifier = 0;
  uint16_t                                ipcp_req_length = 0;
  uint8_t                                 ipcp_req_option = 0;
  uint8_t                                 ipcp_req_option_length = 0;

  uint8_t                                 ipcp_out_code = 0;
  uint16_t                                ipcp_out_length = 0;

  OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP length %u\n", poc_id->length);

  ipcp_req_code = poc_id->contents->data[pco_in_index++];
  ipcp_req_identifier = poc_id->contents->data[pco_in_index++];
  ipcp_req_length = (((uint16_t) poc_id->contents->data[pco_in_index]) << 8) | ((uint16_t) poc_id->contents->data[pco_in_index + 1]);
  OAILOG_TRACE (LOG_SPGW_APP, "PCO: Protocol identifier IPCP (0x%x) code 0x%x identifier 0x%x length %u\n", poc_id->id, ipcp_req_code, ipcp_req_identifier, ipcp_req_length);
  pco_in_index += 2;
  ipcp_req_remaining_length = ipcp_req_remaining_length - 1 - 1 - 2;
  ipcp_out_length = 1 + 1 + 2;

  poc_id_resp.id = poc_id->id;
  poc_id_resp.length = 0; // fill value after parsing req
  uint8_t cil[4] = {0}; // code, identifier, length
  poc_id_resp.contents = blk2bstr(cil, 4); // fill values after parsing req

  while (ipcp_req_remaining_length >= 2) {
    ipcp_req_option = poc_id->contents->data[pco_in_index];
    ipcp_req_option_length = poc_id->contents->data[pco_in_index + 1];
    ipcp_req_remaining_length = ipcp_req_remaining_length - ipcp_req_option_length;
    ipcp_out_code = IPCP_CODE_CONFIGURE_ACK;
    OAILOG_TRACE (LOG_SPGW_APP, "PCO: Protocol identifier IPCP ipcp_option %u ipcp_option_length %u ipcp_remaining_length %lu pco_in_index %lu\n",
        ipcp_req_option, ipcp_req_option_length, ipcp_req_remaining_length, pco_in_index);

    switch (ipcp_req_option) {
      case IPCP_OPTION_PRIMARY_DNS_SERVER_IP_ADDRESS:
        /* RFC 1877
         * This Configuration Option defines a method for negotiating with
         * the remote peer the address of the primary DNS server to be used
         * on the local end of the link. If local peer requests an invalid
         * server address (which it will typically do intentionally) the
         * remote peer specifies the address by NAKing this option, and
         * returning the IP address of a valid DNS server.
         * By default, no primary DNS address is provided.
         */
        OAILOG_TRACE (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option PRIMARY_DNS_SERVER_IP_ADDRESS length %u\n", ipcp_req_option_length);
        if (ipcp_req_option_length >= 6) {
          ipcp_dns_prim_ipv4_addr = (((uint32_t) poc_id->contents->data[pco_in_index + 2]) << 24) |
              (((uint32_t) poc_id->contents->data[pco_in_index + 3]) << 16) |
              (((uint32_t) poc_id->contents->data[pco_in_index + 4]) << 8) |
              (((uint32_t) poc_id->contents->data[pco_in_index + 5]));
          OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option SECONDARY_DNS_SERVER_IP_ADDRESS ipcp_dns_prim_ipv4_addr 0x%x\n", ipcp_dns_prim_ipv4_addr);

          if (ipcp_dns_prim_ipv4_addr == 0) {
            ipcp_out_dns_prim_ipv4_addr = spgw_config.pgw_config.ipv4.default_dns_v4;
            /* RFC 1877:
             * Primary-DNS-Address
             *  The four octet Primary-DNS-Address is the address of the primary
             *  DNS server to be used by the local peer. If all four octets are
             *  set to zero, it indicates an explicit request that the peer
             *  provide the address information in a Config-Nak packet. */
            ipcp_out_code = IPCP_CODE_CONFIGURE_NACK;
          } else if (spgw_config.pgw_config.ipv4.default_dns_v4 != ipcp_dns_prim_ipv4_addr) {
            ipcp_out_code = IPCP_CODE_CONFIGURE_NACK;
            ipcp_out_dns_prim_ipv4_addr = spgw_config.pgw_config.ipv4.default_dns_v4;
          } else {
            ipcp_out_dns_prim_ipv4_addr = ipcp_dns_prim_ipv4_addr;
          }

          OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option SECONDARY_DNS_SERVER_IP_ADDRESS ipcp_out_dns_prim_ipv4_addr 0x%x\n", ipcp_out_dns_prim_ipv4_addr);
        }
        uint8_t idp[6] = {0};
        idp[0] = IPCP_OPTION_PRIMARY_DNS_SERVER_IP_ADDRESS;
        idp[1] = 6;
        idp[2] = (uint8_t) (ipcp_out_dns_prim_ipv4_addr & 0x000000FF);
        idp[3] = (uint8_t) ((ipcp_out_dns_prim_ipv4_addr >> 8) & 0x000000FF);
        idp[4] = (uint8_t) ((ipcp_out_dns_prim_ipv4_addr >> 16) & 0x000000FF);
        idp[5] = (uint8_t) ((ipcp_out_dns_prim_ipv4_addr >> 24) & 0x000000FF);
        ipcp_out_length += 6;
        bcatblk(poc_id_resp.contents, idp, 6);
        break;

      case IPCP_OPTION_SECONDARY_DNS_SERVER_IP_ADDRESS:
        /* RFC 1877
         * This Configuration Option defines a method for negotiating with
         * the remote peer the address of the secondary DNS server to be used
         * on the local end of the link. If local peer requests an invalid
         * server address (which it will typically do intentionally) the
         * remote peer specifies the address by NAKing this option, and
         * returning the IP address of a valid DNS server.
         * By default, no secondary DNS address is provided.
         */
        OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option SECONDARY_DNS_SERVER_IP_ADDRESS length %u\n", ipcp_req_option_length);

        if (ipcp_req_option_length >= 6) {
          ipcp_dns_sec_ipv4_addr = (((uint32_t) poc_id->contents->data[pco_in_index + 2]) << 24) |
              (((uint32_t) poc_id->contents->data[pco_in_index + 3]) << 16) |
              (((uint32_t) poc_id->contents->data[pco_in_index + 4]) << 8) |
              (((uint32_t) poc_id->contents->data[pco_in_index + 5]));
          OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option SECONDARY_DNS_SERVER_IP_ADDRESS ipcp_dns_sec_ipv4_addr 0x%x\n", ipcp_dns_sec_ipv4_addr);

          if (ipcp_dns_sec_ipv4_addr == 0) {
            ipcp_out_dns_sec_ipv4_addr = spgw_config.pgw_config.ipv4.default_dns_sec_v4;
          } else if (spgw_config.pgw_config.ipv4.default_dns_sec_v4 != ipcp_dns_sec_ipv4_addr) {
            ipcp_out_code = IPCP_CODE_CONFIGURE_NACK;
            ipcp_out_dns_sec_ipv4_addr = spgw_config.pgw_config.ipv4.default_dns_sec_v4;
          } else {
            ipcp_out_dns_sec_ipv4_addr = ipcp_dns_sec_ipv4_addr;
          }

          OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option SECONDARY_DNS_SERVER_IP_ADDRESS ipcp_out_dns_sec_ipv4_addr 0x%x\n", ipcp_out_dns_sec_ipv4_addr);
        }
        uint8_t ids[6] = {0};
        ids[0] = IPCP_OPTION_SECONDARY_DNS_SERVER_IP_ADDRESS;
        ids[1] = 6;
        ids[2] = (uint8_t) (ipcp_out_dns_sec_ipv4_addr & 0x000000FF);
        ids[3] = (uint8_t) ((ipcp_out_dns_sec_ipv4_addr >> 8) & 0x000000FF);
        ids[4] = (uint8_t) ((ipcp_out_dns_sec_ipv4_addr >> 16) & 0x000000FF);
        ids[5] = (uint8_t) ((ipcp_out_dns_sec_ipv4_addr >> 24) & 0x000000FF);
        ipcp_out_length += 6;
        bcatblk(poc_id_resp.contents, ids, 6);

        ipcp_out_length += 6;
        break;

      default:
        OAILOG_WARNING (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option 0x%04X unknown\n", ipcp_req_option);
    }
    pco_in_index += ipcp_req_option_length;
  }

  // finally we can fill code, length
  poc_id_resp.length = ipcp_out_length; // fill value after parsing req
  poc_id_resp.contents->data[0] = ipcp_out_code;
  poc_id_resp.contents->data[1] = ipcp_req_identifier;
  poc_id_resp.contents->data[2] = (uint8_t)(ipcp_out_length >> 8);
  poc_id_resp.contents->data[3] = (uint8_t)(ipcp_out_length & 0x00FF);

  return pgw_pco_push_protocol_or_container_id(pco_resp, &poc_id_resp);
}

//------------------------------------------------------------------------------
int pgw_process_pco_dns_server_request(protocol_configuration_options_t * const pco_resp, const pco_protocol_or_container_id_t * const poc_id)
{
  uint32_t                                ipcp_out_dns_prim_ipv4_addr = spgw_config.pgw_config.ipv4.default_dns_v4;
  pco_protocol_or_container_id_t          poc_id_resp = {0};
  uint8_t                                 dns_array[4];

  OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option DNS Server Request length %u\n", poc_id->length);
  poc_id_resp.id = PCO_CI_DNS_SERVER_IPV4_ADDRESS;
  poc_id_resp.length = 4;
  dns_array[0] = (uint8_t) (ipcp_out_dns_prim_ipv4_addr & 0x000000FF);
  dns_array[1] = (uint8_t) ((ipcp_out_dns_prim_ipv4_addr >> 8) & 0x000000FF);
  dns_array[2] = (uint8_t) ((ipcp_out_dns_prim_ipv4_addr >> 16) & 0x000000FF);
  dns_array[3] = (uint8_t) ((ipcp_out_dns_prim_ipv4_addr >> 24) & 0x000000FF);
  poc_id_resp.contents = blk2bstr(dns_array, sizeof(dns_array));

  return pgw_pco_push_protocol_or_container_id(pco_resp, &poc_id_resp);
}
//------------------------------------------------------------------------------
int pgw_process_pco_link_mtu_request(protocol_configuration_options_t * const pco_resp, const pco_protocol_or_container_id_t * const poc_id)
{
  pco_protocol_or_container_id_t          poc_id_resp = {0};
  uint8_t                                 mtu_array[4];

  OAILOG_DEBUG (LOG_SPGW_APP, "PCO: Protocol identifier IPCP option Link MTU Request length %u\n", poc_id->length);
  poc_id_resp.id = PCO_CI_IPV4_LINK_MTU;
  poc_id_resp.length = 2;
  mtu_array[0] = (uint8_t)(1500 >> 8);
  mtu_array[1] = (uint8_t)(1500 &0xFF);
  poc_id_resp.contents = blk2bstr(mtu_array, sizeof(mtu_array));

  return pgw_pco_push_protocol_or_container_id(pco_resp, &poc_id_resp);
}

//------------------------------------------------------------------------------

int pgw_process_pco_request(const protocol_configuration_options_t * const pco_req, protocol_configuration_options_t * pco_resp, bool * const address_allocation_via_nas_signalling)
{
  bool mtu_requested = false;
  bool dns_v4_requested = false;

  *address_allocation_via_nas_signalling = false;

  switch (pco_req->configuration_protocol) {
    case PCO_CONFIGURATION_PROTOCOL_PPP_FOR_USE_WITH_IP_PDP_TYPE_OR_IP_PDN_TYPE:
      pco_resp->ext = 1;
      pco_resp->spare = 0;
      pco_resp->num_protocol_or_container_id = 0;
      pco_resp->configuration_protocol = pco_req->configuration_protocol;
      break;

    default:
      OAILOG_WARNING (LOG_SPGW_APP, "PCO: configuration protocol 0x%X not supported now\n", pco_req->configuration_protocol);
      break;
  }

  int id = 0;
  while (id < pco_req->num_protocol_or_container_id) {

    switch (pco_req->protocol_or_container_ids[id].id) {
    case PCO_PI_IPCP:
      pgw_process_pco_request_ipcp(pco_resp, &pco_req->protocol_or_container_ids[id]);
      break;

    case PCO_CI_DNS_SERVER_IPV4_ADDRESS_REQUEST:
      pgw_process_pco_dns_server_request(pco_resp, &pco_req->protocol_or_container_ids[id]);
      dns_v4_requested = true;
      break;

    case PCO_CI_IPV4_LINK_MTU_REQUEST:
      pgw_process_pco_link_mtu_request(pco_resp, &pco_req->protocol_or_container_ids[id]);
      mtu_requested = true;
      break;

    case PCO_CI_IP_ADDRESS_ALLOCATION_VIA_NAS_SIGNALLING:
      *address_allocation_via_nas_signalling = true;
      break;

    default:
      OAILOG_WARNING (LOG_SPGW_APP, "PCO: Protocol identifier 0x%X not supported now\n", pco_req->protocol_or_container_ids[id].id);
    }
    id++;
  }

  if (spgw_config.pgw_config.force_push_pco) {
    *address_allocation_via_nas_signalling = true;
    // TODO tweak
    if (!dns_v4_requested) {
      pgw_process_pco_dns_server_request(pco_resp, NULL);
    }
    if (!mtu_requested) {
      pgw_process_pco_link_mtu_request(pco_resp, NULL);
    }
  }
  return RETURNok;
}
