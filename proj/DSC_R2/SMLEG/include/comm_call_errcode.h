/*******************************************************************************
 * AUTHOR 		: BY JUNE.
 * DATE			: 2011-05-09
 * PROJECT 		: LGT_DSC
 * FILE 		: comm_call_errcode.h
 * RECOMMAND	: 호처리 프로세스들이 사용하며 호 처리 예외 상황에 대한 error code 정의.
*******************************************************************************/

#ifndef __COMM_CALL_ERRCODE_H
#define __COMM_CALL_ERRCODE_H

/* CALL CONTROL ERROR CODE */
/* 1. account-request(start) */
#define ERR_10001                           -10001          // p/h bit escapes a scope  
#define ERR_10002                           -10002          // ruleset used flag off
#define ERR_10003                           -10003          // not match service option
#define ERR_10004                           -10004          // not exist p/hbit at rulefile
#define ERR_10005                           -10005          // ruleset is negative value
#define ERR_10006                           -10006          // ruleset equal
#define ERR_10007                           -10007          // cps over
#define ERR_10008                           -10008          // session not created
#define ERR_10009                           -10009          // session not found
#define ERR_10010                           -10010          // route to RLEG fail
#define ERR_10011                           -10011          // send to RLEG fail
#define ERR_10012                           -10012          //
 
/* 2. account-request(interim) */
#define ERR_20001                           -20001          // p/h bit escapes a scope
#define ERR_20002                           -20002          // ruleset used flag off
#define ERR_20003                           -20003          // not match service option
#define ERR_20004                           -20004          // not exist p/hbit at rulefile
#define ERR_20005                           -20005          // ruleset is negative value
#define ERR_20006                           -20006          // ruleset equal
#define ERR_20007                           -20007          // cps over
#define ERR_20008                           -20008          // session not created
#define ERR_20009                           -20009          // session not found
#define ERR_20010                           -20010          // route to RLEG fail
#define ERR_20011                           -20011          // send to RLEG fail
#define ERR_20012                           -20012

/* 3. account-request(stop) */
#define ERR_30001                           -30001          // p/h bit escapes a scope
#define ERR_30002                           -30002          // ruleset used flag off
#define ERR_30003                           -30003          // not match service option
#define ERR_30004                           -30004          // not exist p/hbit at rulefile
#define ERR_30005                           -30005          // ruleset is negative value
#define ERR_30006                           -30006          // ruleset equal
#define ERR_30007                           -30007          // cps over
#define ERR_30008                           -30008          // session not created
#define ERR_30009                           -30009          // session not found                                                                                            
#define ERR_30010                           -30010          // route to RLEG fail                                                                                           
#define ERR_30011                           -30011          // send to RLEG fail                                                                                            
#define ERR_30012                           -30012          // used flag zero and session not found                                                                         
#define ERR_30013                           -30013          // used flag zero and non-logon session                                                                         
#define ERR_30014                           -30014          // not exist p/hbit at rulefile                                                                                 
                                                                                                                                                                            
/* 4. disconnect-request */                                                                                                                                                 
#define ERR_40001                           -40001          // p/h bit escapes a scope                                                                                      
#define ERR_40002                           -40002          // ruleset used flag off                                                                                        
#define ERR_40003                           -40003          // not match service option                                                                                     
#define ERR_40004                           -40004          // not exist p/hbit at rulefile                                                                                 
#define ERR_40005                           -40005          // ruleset is negative value                                                                                    
#define ERR_40006                           -40006          // ruleset equal                                                                                                
#define ERR_40007                           -40007          // cps over                                                                                                     
#define ERR_40008                           -40008          // session not created                                                                                          
#define ERR_40009                           -40009          // session not exist                                                                                            
#define ERR_40010                           -40010          // route to RLEG fail                                                                                           
#define ERR_40011                           -40011          // send to RLEG fail                                                                                            
#define ERR_40012                           -40012          // hbit attribute not found                                                                                     
#define ERR_40013                           -40013          // hbit equal

/* 5. session timeout */                                                                                                                                                    
#define ERR_50001                           -50001          // p/h bit escapes a scope                                                                                      
#define ERR_50002                           -50002          // ruleset used flag off                                                                                        
#define ERR_50003                           -50003          // not match service option                                                                                     
#define ERR_50004                           -50004          // not exist p/hbit at rulefile                                                                                 
#define ERR_50005                           -50005          // ruleset is negative value                                                                                    
#define ERR_50006                           -50006          // ruleset equal                                                                                                
#define ERR_50007                           -50007          // cps over                                                                                                     
#define ERR_50008                           -50008          // session not created                                                                                          
#define ERR_50009                           -50009          // session not found                                                                                            
#define ERR_50010                           -50010          // route to RLEG fail                                                                                           
#define ERR_50011                           -50011          // send to RLEG fail                                                                                            
#define ERR_50012                           -50012          // system mode is standby                                                                                       
#define ERR_50013                           -50013          // system mode is fault                                                                                         
#define ERR_50014                           -50014          // not logon session
#define ERR_50015                           -50015          // logon operation-mode invaild

#endif	//__COMM_CALL_ERRCODE_H

