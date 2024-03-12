-- All solutions
SELECT tblsolution.*,
       tblsolution.codecatsolution AS numcatsolution,
       nomcatsolution,
       tbltechno.numtechno,
       tbltechno.imgtechno,
       tblnomtechno.nomtechno
FROM tblsolution
         LEFT JOIN(SELECT codeappelobjet,
                          traductiondictionnairecategories AS nomcatsolution
                   FROM tbldictionnairecategories
                   WHERE typedictionnairecategories = 'solcat'
                     AND codelangue = "2") tblnomcatsolution
                  ON
                      tblnomcatsolution.codeappelobjet = tblsolution.codecatsolution
         LEFT JOIN tbltechno ON tbltechno.numtechno = tblsolution.codetechno
         LEFT JOIN(SELECT codeappelobjet,
                          traductiondictionnaire AS nomtechno
                   FROM tbldictionnaire
                   WHERE typedictionnaire = 'tec'
                     AND codelangue = "2") tblnomtechno
                  ON
                      tblnomtechno.codeappelobjet = tbltechno.numtechno

-- Description all solutions
SELECT indexdictionnaire, traductiondictionnaire
FROM tbldictionnaire
WHERE typedictionnaire = 'sol'
  AND codelangue = "2"