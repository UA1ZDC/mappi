!	 model vlazhnosty na osnove tablitc GOST 26352-84.
		SUBROUTINE VLAZH(lat,lon,p,MMDD,h,eh)
		REAL jan0(11,10),jul0(11,10),jan80(11,10),jul80(11,10),h(100),kof04
		REAL jan180(11,10),jul180(11,10),jan280(11,10),jul280(11,10)
		REAL merid(4), lati(10),p(100),lat,lon,jan(11,10),jul(11,10)
		REAL janu(11),july(11),ejan(11),ejul(11),eh(100),pi,a,b
                REAL tmp
 		INTEGER	i,j,k1,k2,MMDD,no
!		COMMON/VLA/jan0,jul0,jan80,jul80,jan180,jul180,jan280,jul280
!====================================
		pi=3.14159
       OPEN(10,FILE='0_jan.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(10,*)jan0(i,j)
	END DO
      END DO
      CLOSE(UNIT=10)
       OPEN(11,FILE='0_july.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(11,*)jul0(i,j)
	END DO
      END DO
      CLOSE(UNIT=11)
       OPEN(12,FILE='80_jan.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(12,*)jan80(i,j)
	END DO
      END DO
      CLOSE(UNIT=12)
       OPEN(13,FILE='80_july.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(13,*)jul80(i,j)
	END DO
      END DO
      CLOSE(UNIT=13)
       OPEN(14,FILE='180_jan.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(14,*)jan180(i,j)
	END DO
      END DO
      CLOSE(UNIT=14)
       OPEN(15,FILE='180_july.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(15,*)jul180(i,j)
	END DO
      END DO
      CLOSE(UNIT=15)
       OPEN(16,FILE='280_jan.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(16,*)jan280(i,j)
	END DO
      END DO
      CLOSE(UNIT=16)
       OPEN(17,FILE='280_july.dat',STATUS='OLD') 
	DO j=1,10
      DO i=1,11 
      READ(17,*)jul280(i,j)
	END DO
      END DO
      CLOSE(UNIT=17)
!-----------------------------------------
		DO i=1,11  !visota
		h(i)=REAL(i-1)
		END DO
! -----------------------------------------
		merid(1)=0.  !profilnie meridiany
		merid(2)=80.
		merid(3)=180.
		merid(4)=280.
! --------------------------------------
		DO i=1,10
		lati(i)=REAL(i-1)*10.
		END DO
! --------poisk meridiana-----------------
		k1=0
		DO i=1,3
		IF((merid(i).LE.lon).AND.(merid(i+1).GE.lon))THEN
		k1=i
		k2=i+1
		GOTO 135
		END IF 
		END DO
135    CONTINUE
		IF((k1.EQ.1).AND.(k2.EQ.2)) THEN
! ====================profili vdol zadannogo meridiana==============
		tmp=lon/80.
		DO j=1,10
		DO i=1,11 	
		jan(i,j)=jan0(i,j)+(jan80(i,j)-jan0(i,j))*tmp
		jul(i,j)=jul0(i,j)+(jul80(i,j)-jul0(i,j))*tmp
		END DO     	
		END DO     
		ELSE IF((k1.EQ.2).AND.(k2.EQ.3)) THEN 
		tmp=(lon-80.)/(180.-80.)
		DO j=1,10
		DO i=1,11 	
		jan(i,j)=jan80(i,j)+(jan180(i,j)-jan80(i,j))*tmp
		jul(i,j)=jul80(i,j)+(jul180(i,j)-jul80(i,j))*tmp
		END DO
		END DO
		ELSE IF((k1.EQ.3).AND.(k2.EQ.4)) THEN 
		tmp=(lon-180.)/(280.-180.)
		DO j=1,10
		DO i=1,11 	
		jan(i,j)=jan180(i,j)+(jan280(i,j)-jan180(i,j))*tmp
		jul(i,j)=jul180(i,j)+(jul280(i,j)-jul180(i,j))*tmp
		END DO
		END DO
		ELSE IF(k1.EQ.0) THEN 
		tmp=(lon-280.)/(360.-280.)
		DO j=1,10
		DO i=1,11 	
		jan(i,j)=jan280(i,j)+(jan0(i,j)-jan280(i,j))*tmp
		jul(i,j)=jul280(i,j)+(jul0(i,j)-jul280(i,j))*tmp
		END DO
		END DO
		END IF
!=======================profili for zadannoi po modulu shiroty 
! 	DO i=1,18
		DO i=1,9
		IF((ABS(lat).GE.lati(i)).AND.(ABS(lat).LE.lati(i+1))) THEN
		k1=i
		k2=i+1
		tmp=(lat-lati(i))/10.
		GOTO 140
		END IF
		END DO
140		DO i=1,11
		  janu(i)=jan(i,k1)+(jan(i,k2)-jan(i,k1))*tmp
		  july(i)=jul(i,k1)+(jul(i,k2)-jul(i,k1))*tmp
		END DO
!+++++++++++++++++++++++++++++++++++++
		DO i=1,11  ! v GPa !!!!!!
		ejan(i)=janu(i)*p(i)/(621.98+0.378*janu(i))/100.
		ejul(i)=july(i)*p(i)/(621.98+0.378*july(i))/100.
		END DO
!======================================
		no=mmdd/100 !nomer mesyatca
		IF(LAT.GT.0) THEN  !for Noth polushariya
		DO i=1,11
		a=0.5*(ejan(i)+ejul(i))
		b=1.1547*(ejan(i)-ejul(i))/2.
   		eh(i)=a+b*COS(pi/6.*(REAL(no)-2.))
		END DO
		ELSE IF(lat.LT.0) THEN
		DO i=1,11
		a=0.5*(ejan(i)+ejul(i))
		b=1.1547*(ejul(i)-ejan(i))/2.
		eh(i)=a+b*COS(pi/6.*(REAL(no)-2.))
		END DO
		END IF
		RETURN
		END
