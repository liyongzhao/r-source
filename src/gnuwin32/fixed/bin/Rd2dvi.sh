#
#  Rd2dvi -- check man pages (*.Rd help files) by LaTeX..
#
# Examples:
#  R CMD Rd2dvi /path/to/Rsrc/src/library/base/man/Normal.Rd
#  R CMD Rd2dvi `grep -l "\\keyword{distr" \
#                  /path/to/Rsrc/src/library/base/man/*.Rd | sort | uniq`

R_PAPERSIZE=${R_PAPERSIZE:-a4}

revision='$Revision: 1.1.2.1 $'
version=`set - ${revision}; echo ${2}`
version="Rd2dvi ${version}" 

usage="Usage: R CMD Rd2dvi [options] files

Generate DVI (or PDF) output from the Rd sources specified by files, by
either giving the paths to the files, or the path to a directory with
the sources of a package.

Options:
  --debug		turn on shell debugging (set -x)
  -h, --help		print short help message and exit
  --no-clean		do not remove created temporary files
  --no-preview		do not preview generated output file
  --output=FILE		write output to FILE
  --pdf			generate PDF output
  --title=NAME		use NAME as the title of the document
  -v, --version		print version info and exit
  -V, --verbose		report on what is done

Report bugs to <R-bugs@lists.r-project.org>."

start_dir=`pwd`

clean=true
debug=false
dvi_ext="dvi"
output=""
preview=${xdvi:-xdvi.bat}
verbose=false

TEXINPUTS=.:${R_HOME}/doc/manual:${TEXINPUTS}

file_sed='s/[_$]/\\\\&/g'

while test -n "${1}"; do
  case ${1} in
    -h|--help)
      echo "${usage}"; exit 0 ;;
    -v|--version)
      echo "${version}"; exit 0 ;;
    --debug)
      debug=true ;;
    --no-clean)
      clean=false ;;
    --no-preview)
      preview=false ;;
    --pdf)
      dvi_ext="pdf";
      preview=false;
      R_RD4DVI=${R_RD4PDF:-"ae,hyper"};
      R_LATEXCMD=${PDFLATEX:-pdflatex};;
    --title=*)
      title=`echo "${1}" | sed -e 's/[^=]*=//'` ;;
    --output=*)
      output=`echo "${1}" | sed -e 's/[^=]*=//'` ;;
    -V|--verbose)
      verbose=echo ;;
    --|*)
      break ;;
  esac
  shift
done

if test -z "${output}"; then
  output=Rd2.${dvi_ext}
fi

if ${debug}; then set -x; fi

toc="\\Rdcontents{\\R{} objects documented:}"
if [ ${#} -eq 1 -a -d ${1} ]; then
  if test -d ${1}/man; then
    dir=${1}/man
  else
    dir=${1}
  fi
  ${verbose} "Rd2dvi: \`Rdconv -t latex' ${dir}/ Rd files ..."
  files=`find ${dir} -name "*.[Rr]d" -print`
  subj="all in \\file{`echo ${dir} | sed ${file_sed}`}"
  ${verbose} "files = ${files}"
else
  files="${@}"
  if [ ${#} -gt 1 ]; then
    subj=" etc.";
  else
    subj=
    toc=
  fi
  subj="\\file{`echo ${1} | sed ${file_sed}`}${subj}"
  ${verbose} "Rd2dvi: Collecting files and \`Rdconv -t latex'ing them ..."
fi

${verbose} ""
${verbose} "subj: ${subj}"
${verbose} ""

if test -f ${output}; then
  echo "file \`${output}' exists; please remove first"
  exit 1
fi
build_dir=.Rd2dvi${$}
if test -d ${build_dir}; then
  rm -rf ${build_dir} || echo "cannot write to build dir" && exit 2
fi
mkdir ${build_dir}

sed 's/markright{#1}/markboth{#1}{#1}/' \
  ${R_HOME}/doc/manual/Rd.sty > ${build_dir}/Rd.sty

title=${title:-"\\R{} documentation}} \\par\\bigskip{{\\Large of ${subj}"}

cat > ${build_dir}/Rd2.tex <<EOF
\\documentclass[${R_PAPERSIZE}paper]{book}
\\usepackage[${R_RD4DVI:-ae}]{Rd}
\\usepackage{makeidx}
\\makeindex
\\begin{document}
\\chapter*{}
\\begin{center}
{\\textbf{\\huge ${title}}}
\\par\\medskip{\\large \\today}
\\end{center}
${TOC}
EOF

for f in ${files}; do
  ${verbose} ${f}
  ${R_HOME}/bin/Rdconv.bat -t latex ${f} >> ${build_dir}/Rd2.tex
done

cat >> ${build_dir}/Rd2.tex <<EOF
\\printindex
\\end{document}
EOF

cd ${build_dir}
${R_LATEXCMD:-latex} Rd2
${R_MAKEINDEXCMD:-makeindex} Rd2
${R_LATEXCMD:-latex} Rd2
cp Rd2.${dvi_ext} ../${output}
cd ${start_dir}
if ${clean}; then
  rm -rf ${build_dir}
else
  echo "You may want to clean up by \`rm -rf ${build_dir}'"
fi
${preview} ${output}
exit 0

### Local Variables: ***
### mode: sh ***
### sh-indentation: 2 ***
### End: ***