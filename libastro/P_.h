/* surround declarations with P_() to include prototypes if it looks likes
 * compiler should handle them.
 */
#ifndef P_
#if defined(__STDC__) || defined(__cplusplus) || NeedFunctionPrototypes
#define P_(s) s
#else
#define P_(s) ()
#endif
#endif /* P_ */

/* For RCS Only -- Do Not Edit
 * @(#) $RCSfile: P_.h,v $ $Date: 2003/03/04 05:44:05 $ $Revision: 1.2 $ $Name:  $
 */
