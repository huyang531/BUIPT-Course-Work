#!/bin/bash
# This program helps you manage your files



################### Search program starts from here ###################
function search()
{
	if [[ $# == 0 ]]; then
		# Welcome
		clear
		echo "Welcome!"
		echo "--------"
		echo "1) Start searching"
		echo "2) Back to index page"
		echo ""

		while true
		do
			read -p "Enter your choice: " start
			case $start in
			1 ) break
				;;
			2 ) index
				;;
			* ) echo "Invalid input!"
				;;
			esac
		done
	fi

	# Search type?
	if [[ -z $1 || $1 == 5 || $1 == 7 ]]; then
		clear
		echo "What do you want to search for?"
		echo "-------------------------------"
		echo "1) File"
		echo "2) Directory"
		echo ""

		while true
			do		
				read -p "Enter your choice: " type
				case $type in
					1 ) type="f"
					    break
					    ;;
					2 ) type="d"
					    break
					    ;;
					* ) echo "Invalid input!"
						;;
				esac
			done
	else
		type=f
	fi

	# under current dir?
	clear
	while true
	do
		read -p "Under current directory? (y/n): " dir
		if [[ -z $dir ]]; then
			dir=y
		fi
		case $dir in
			y ) break
				;;
			n ) while true
				do
					read -p "Redirect to: " newDir
					if [[ -d $newDir ]]; then
						cd $newDir
						break
					else
						echo "Input is not a directory!"
					fi
				done
				break
				;;
			* ) echo "Input must be 'y' or 'n'!"
				;;
		esac
	done

	# prompt for more info when user is not to batch rename files
	if [[ $1 -ne 3 ]]; then
		# search depth?
		clear
		while true
		do
			read -p "Enter max. search depth: " depth
			if [[ "$depth" =~ ^[0-9]+$ ]]; then
				break
			elif [[ -z "$depth" ]]; then
				depth=1
				break;
			else
				echo "Input is not an integer!"
			fi
		done

		# output mode?
		clear
		echo "Choose output mode:"
		echo "-------------------"
		echo "1) Ascending"
		echo "2) Descending"
		echo ""

		while true
		do
			read -p "Enter your choice: " output
			if [[ $output == 1 || $output == 2 ]]; then
				break
			elif [[ -z $output ]]; then
				output=1
				break
			else
				echo "Invalid input!"
			fi
		done
	else
		depth=1
		output=1
	fi

	# redirect to different functions
	case $type in
		f ) searchFile $1
			;;
		d ) searchDir $1
			;;
	esac
}

function searchFile()
{
	# search by?
	clear

	while true
	do
		echo "Search file by?"
		echo "---------------"
		echo "1) name"
		echo "2) last accessed time"
		echo "3) last modified time"
		echo "4) last changed time"
		echo "5) size"
		echo "6) show me all file(s) under this directory and prompt me again"
		echo "7) skip and show default result"
		echo ""
		
		read -p "Enter your choice: " by
		case $by in
			1 ) searchByName
				break
				;;
			2 ) searchByAtime
				break
				;;
			3 ) searchByMtime
				break
				;;
			4 ) searchByCtime
				break
				;;
			5 ) searchBySize
				break
				;;
			6 ) find . -maxdepth $depth -type f
				echo ""
				;;
			7 ) find . -maxdepth $depth -type f -size +10M
				echo ""
				break
				;;
			* ) echo -e "Invalid input!\n" 
				;;
		esac
	done

	case $1 in
		3 ) rename $1 
			;;
		5 ) copy f
			;;
		7 ) delete f
			;;
	esac

	echo "Search result has been saved to files.txt"
	read -n 1 -s -r -p "Press any key to go back to index..."
	index
}

function searchDir()
{
	# search by?
	clear
	while true
	do
		echo "Choose one:"
		echo "-----------"
		echo "1) search directory by name"
		echo "2) search directory by last modified time"
		echo "3) search directory by last changed time"
		echo "4) search directory by last accessed time"
		echo "5) show me all directory(ies) under this directory and prompt me again"
		echo "6) skip and show default result"
		echo ""
		
		read -p "Enter your choice: " by
		case $by in
			1 ) searchByName
				break
				;;
			2 ) searchByMtime
				break
				;;
			3 ) searchByCtime
				break
				;;
			4 ) searchByAtime
				break
				;;
			5 ) find . -maxdepth $depth -type d
				echo ""
				;;
			6 ) find . -maxdepth $depth -type d
				echo ""
				break
				;;
			* ) echo -e "Invalid input!\n" 
				;;
		esac
	done

	case $1 in
	5 ) copy d
		;;
	7 ) delete d
		;;
	esac

	echo "Search result has been saved to directories.txt"
	read -n 1 -s -r -p "Press any key to go back to index..."
	index
}

function searchByName()
{	
	clear
	echo "Note: you can use * for ambiguious search;"
	echo "      e.g. '*.py' finds all files end with '.py'"
	echo ""

	# name?
	while true
	do
		read -p "Enter name of file/directory: " name
		if [[ -z "$name" ]]; then
			echo "Empty input!"
		else
			break
		fi
	done

	# case-sensitive?
	while true
	do
		read -p "Is your name case-sensitive? (y/n): " case
		# show result
		case $case in
			y ) echo -e "\nSearch Result:"
				output 1
				echo ""
				break
				;;
			n ) echo -e "\nSearch Result:"
				output 2
				echo ""
				break
				;;
			* ) echo "Invalid input!"
				;;
		esac
	done
}

function searchBySize()
{
	clear
	echo "Note: use '-' for smaller than and '+' for larger than;"
	echo "      use 'M' for MegaByte and 'k' for KiloByte;"
	echo "      e.g. '-10M' stands for smaller than 10 MegaByte"
	echo ""

	# size limit?
	while true
	do
		read -p "Enter size limit: " size
		if [[ ! -z "$(echo "$size" | grep '^[\+|\-][0-9]\+[M|k]$')" ]]; then
			# show result
			echo -e "\nSearch Result:"
			output 3
			echo ""
			break
		else
			echo "Invalid input!"			
		fi
	done
}

function setTime()
{
	# limit time by?
	clear
	echo "Limit time by?"
	echo "--------------"
	echo "1) Minute"
	echo "2) Day"
	echo ""

	while true
	do
		read -p "Enter your choice: " limit
		if [[ $limit == 1 || $limit == 2 ]]; then
			break;
		else
			echo "Invalid input!"
		fi
	done

	# time limit?
	clear
	echo "Note: use '-' for less than and '+' for more than;"
	echo "      e.g. '+2' stands for more than 2 days/minutes ago"
	case limit in
		1 ) echo "      Current metric: minute"
			;;
		2 ) echo "      Current metric: day"
			;;
	esac
	echo ""

	while true
	do
		read -p "Enter time limit: " time
		if [[ ! -z "$(echo "$time" | grep -i '^[\+|\-][0-9]\+$')" ]]; then
			break
		else
			echo "Invalid input!"
		fi
	done
}

function searchByMtime()
{	
	setTime
	
	# show result
	case $limit in
		1 ) echo -e "\nSearch Result:"
			output 4
			echo ""
			;;
		2 ) echo -e "\nSearch Result:"
			output 5
			;;
	esac
}

function searchByCtime()
{
	setTime

	# show result
	case $limit in
		1 ) echo -e "\nSearch Result:"
			output 6
			echo ""
			;;
		2 ) echo -e "\nSearch Result:"
			output 7
			;;
	esac
}

function searchByAtime()
{
	setTime

	# show result
	case $limit in
		1 ) echo -e "\nSearch Result:"
			output 8
			echo ""
			;;
		* ) echo -e "\nSearch Result:"
			output 9
			;;
	esac
}

function output()
{
	case $1 in
		1 ) if [[ $type == f ]]; then
			echo "Filename"
			else
			echo "Directory"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -name "$name" -printf "%p\n" | sort | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -name "$name" -printf "%p\n" | sort -r | tee sorted.txt
			fi
			;;
		2 ) if [[ $type == f ]]; then
			echo "Filename"
			else
			echo "Directory"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -iname "$name" -printf "%p\n" | sort | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -iname "$name" -printf "%p\n" | sort -r | tee sorted.txt
			fi
			;;
		3 ) echo -e "Filename\tSize(in bytes)"
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type f -size "$size" -printf "%p\t%s\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type f -size "$size" -printf "%p\t%s\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
		4 ) if [[ $type == f ]]; then
				echo -e "Filename\tLast modified"
			else
				echo -e "Directory\tLast modified"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -mmin $time -printf "%p\t%T+\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -mmin $time -printf "%p\t%T+\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
		5 ) if [[ $type == f ]]; then
				echo -e "Filename\tLast modified"
			else
				echo -e "Directory\tLast modified"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -mtime $time -printf "%p\t%T+\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -mtime $time -printf "%p\t%T+\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
		6 ) if [[ $type == f ]]; then
				echo -e "Filename\tLast changed"
			else
				echo -e "Directory\tLast changed"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -cmin $time -printf "%p\t%C+\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -cmin $time -printf "%p\t%C+\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
		7 ) if [[ $type == f ]]; then
				echo -e "Filename\tLast changed"
			else
				echo -e "Directory\tLast changed"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -ctime $time -printf "%p\t%C+\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -ctime $time -printf "%p\t%C+\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
		8 ) if [[ $type == f ]]; then
				echo -e "Filename\tLast accessed"
			else
				echo -e "Directory\tLast accessed"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -amin $time -printf "%p\t%A+\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -amin $time -printf "%p\t%A+\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
		9 ) if [[ $type == f ]]; then
				echo -e "Filename\tLast accessed"
			else
				echo -e "Directory\tLast accessed"
			fi
			if [[ $output == 1 ]]; then
				find . -maxdepth $depth -type $type -atime $time -printf "%p\t%A+\n" | sort -k 2n | tee sorted.txt
			else
				find . -maxdepth $depth -type $type -atime $time -printf "%p\t%A+\n" | sort -k 2n -r | tee sorted.txt
			fi
			;;
	esac

	# # following is for testing purpose, manually remove cut1.txt, sorted.txt afterwards!
	# cut -d "	" -f 1 sorted.txt > cut1.txt
	# # filter out system files
	# grep -v "^\./\." cut1.txt > files.txt

	# use this instead:!!!!!!
	if [[ $1 == 1 || $1 == 2 ]]; then
		grep -v "^\./\." sorted.txt > files.txt
	else
		cut -d "	" -f 1 sorted.txt > sorted_cut.txt
		# filter out system files
		grep -v "^\./\." sorted_cut.txt > files.txt
	fi
	rm sorted_cut.txt
	rm sorted.txt

	if [[ $type == d ]]; then
		mv files.txt directories.txt
	fi
}
############################# Search program ends here #############################



############################# Rename program starts from here ##########################
function rename()
{
	if [[ $1 == 3 ]]; then
		renameFile
	fi

	clear
	echo "What do you want to batch rename?"
	echo "---------------------------------"
	echo "1) File"
	echo "2) Directory"
	echo ""

	while true
	do
		read -p "Enter your choice: " rename_type
		case $rename_type in
			1 ) break
				;;
			2 ) renameDir
				;;
			* ) echo "Invalid input!"
		esac
	done

	# following should be part of renameFile()
	#是否在当前文件夹下?
	clear
	while true
	do
		read -p "In current directory? (y/n): " dir
		if [[ -z $dir ]]; then
			dir=y
		fi
		case $dir in
			y ) break
				;;
			n ) while true
				do
					read -p "Redirect to: " newDir
					if [[ -d $newDir ]]; then
						cd $newDir
						break
					else
						echo "Input is not a directory!"
					fi
				done
				break
				;;
			* ) echo "Input must be 'y' or 'n'!"
				;;
		esac
	done

	#输入数量
	clear
	while true
	do
		read -p "No. of files to be renamed: " num
		if [[ $num =~ ^[0-9]+$ ]]; then
			break
		else
			echo "Input must be a integer!"
		fi
	done

	#创建文件存储旧文件名
	if [[ ! -z `find . -name "files.txt"` ]]; then
		rm files.txt
	fi

	#输入、保存文件名
	i=0
	touch files.txt
	echo "Now enter filenames..."
	while [ $i -ne $num ]
	do
		x=`expr $i + 1`
		while true
		do
			read -p "File #$x: " file
			#判断文件是否存在或重复
			if [[ -f $file && -z `grep "$file" files.txt` ]]; then
				break
			else
				echo "Invalid input!"
			fi
		done
		echo "$file" >> files.txt
		let i++
	done

	renameFile
}

function renameFile()
{
	echo "You are to rename the following files:"
	cat files.txt

	# continue?
	while true
	do
		read -p "Continue? (y/n): " start_renameFile
		if [[ $start_renameFile == y ]]; then
			break
		elif [[ $start_renameFile == n ]]; then
			rm files.txt
			read -n 1 -s -r -p "Press any key to go back to index..."
			index
		else
			echo "Invalid input!"
		fi
	done

	#判断是否有拓展名
	clear
	echo "Note: renamed files may have an extension"
	echo "      e.g. enter 'test.txt' and you get 'test-#.txt'"
	read -p "Enter new filename: " full_new_name
	if [[ $full_new_name =~ '.' ]]; then
		new_extension=".${full_new_name##*.}"
		new_name="${full_new_name%.*}"
	else
		new_extension=""
		new_name="$full_new_name"
	fi

	#将重命名后的文件放在新文件夹
	clear
	mkdir ./$new_name 2> /dev/null
	echo "Renamed files have been saved to ./$new_name"
	
	count=1
	for i in `cat files.txt`
	do
		#添加相同扩展名
	    mv $i ./$new_name/$new_name-$count$new_extension 2> /dev/null
		let count++
	done
	rm files.txt

	echo "Rename done!"
	read -n 1 -s -r -p "Press any key to go back to index..."
	index
}

function renameDir()
{
	#开始
	clear
	echo "Choose one:"
	echo "-----------"
	echo "1) Batch rename directories"
	echo "2) Return to index"
	echo ""

	while true
	do
	   read -p "Enter your choice:" A
	   case $A in
		1 ) break
			;;
		2 ) index
			;;
		* ) echo "Invalid input!"
			;;
	   esac
	done

	#输入修改前后的目录名称
	clear
	echo "Note: you can start with ./ to refer to the current directory"
	echo ""
	read -p "Directory to rename: " olddir
	while [ ! -d $olddir ]
	do
		clear
		echo "Can't find this directory!"
		echo "--------------------------"
		echo "1) See available directory"
		echo "2) Retry"
		echo "3) Exit"
		echo ""
		while true
		do
			read -p "Enter your choice:" B
			case $B in
				1)echo -e "\nAbailable directories:"
					ls -R
					echo ""
					echo "--------------------------"
					echo "1) See available directory"
					echo "2) Retry"
					echo "3) Exit"
					echo ""
					;;
				2)clear
					echo "Note: you can start with ./ to refer to the current directory"
					echo ""
					read -p "Directory to rename: " olddir
	                          break
				    ;;
				3)exit 0
				    ;;                  
	            *)echo "Invalid input!"
	                            ;;
	                esac
		done	
	done

	#跳转到待修改的目录
	cd $olddir
	clear
	read -p "Enter new name: " newdir
	echo "$olddir" >> path.txt
	echo "$newdir" >> newpath.txt

	#替换分隔符
	IFS_old=$IFS
	IFS=$'\/'

	#将各个文件夹的名称保存到数组
	count1=0
	for i in `cat path.txt`
	do
	  let count1++
	  old[$count1]=$i
	done
	count2=0
	for i in `cat newpath.txt`
	do
	  let count2++
	  new[$count2]=$i
	done
	IFS=$IFS_old
	rm path.txt
	rm newpath.txt

	#判断修改后的名称是否对应文件夹个数
	if [ $count1 -ne  $count2 ];
	then
		echo "Error in newname!"
		exit 0
	else
		i=$count2
		#依次跳转上级目录并改名
		while [ $i -gt 1 ]
	        do
		    #筛选需要修改的文件夹进行改名
		    #避免mv报错
		    if [ ${old[$i]} != ${new[$i]} ];
		    then
		        mv ../${old[$i]} ../${new[$i]}
	        fi
		    cd ..
		    let i--
	        done
	fi

	clear
	echo "Rename done!"
	read -n 1 -s -r -p "Press any key to go back to index..."
	index
}
############################# Rename program ends here ##########################



############################ Copy program starts from here ######################
function copyFile()
{
    for file in ${2}/*
    do
        if test -f ${file}
        then
            if [ "${1##*/}" == "${file##*/}" ]; then
            i=`test_whether_same_directory ${1%/*} ${2}`
                if [[ $i == 0 ]]; then
                    rename_while_copy ${1} ${2}
                else
                    echo "The target directory has already contained a file named ${1##*/}"
                    same_name ${1} ${2}            
                fi
            return
            fi
        fi
    done
    cp -p ${1} ${2}${1##*/}
    echo "Successfully done!"
}

function test_whether_same_directory()
{
    if [ $1 == $2 ]; then
        return 0
    else
        return 1
    fi
}

function rename_while_copy()
{
    i=1
    t=${1##*/}
    for file in $2/*
    do
        if test -f ${file}
        then
            if [ "${t%.*}(${i}).${1##*.}" == "${file##*/}" ]
            then
                i=`expr $i + 1`
            fi
        fi
    done
    cp -p $1 "$2/${t%.*}(${i}).${1##*.}"
    echo yes
}

function same_name()
{
    while true
    do
        echo "There is another $1 in `pwd`. What do you want to do?"
        echo "-----------------------------------------------------"
        echo "1) Overwrite"
        echo "2) Rename"
        echo "3) Compare contents"
        echo "4) exit"
        echo 
        read -p "Enter your choice!" i
        case $i in
            1 ) cp -f $1 $2
                break
                ;;
            2 ) rename_while_copy $1 $2
                break
                ;;
            3 ) 
                echo "Content of $1"
                cat $1
                echo
                echo "Content of $2/${1##*/}"
                cat $2/${1##*/}
                ;;
            4 )
                exit 0
                ;;
            * )
                echo "Invalid input!"
                ;;
        esac
    done
}

function copyDirectory()
{
    for directory in ${2}/*
    do
        if test -d ${directory}
        then
            if [ "${1##*/}" == "${directory##*/}" ]; then
            i=`test_whether_same_directory ${1%/*} ${2}`
                if [[ $i == 0 ]]; then
                    echo "The target directory has already contained a directory named ${1##*/}"
                else
                    echo "The target directory has already contained a directory named ${1##*/}"
                    same_name_d ${1} ${2}            
                fi
            return
            fi
        fi
    done
    cp -r ${1} ${2}${1##*/}
    echo "Successfully done!"    
}

function same_name_d()
{
    for FOD in $1/*
    do
        if test -f $FOD
        then
            copyFile $FOD $2
        elif test -d $FOD
        then
            copyDirecotry $FOD $2
        fi
    done
}

function copy()
{
	if [[ -z $1 ]]; then
		# Welcome
		clear
		echo "Welcome!"
		echo "--------"
		echo "1) Start copying"
		echo "2) Return to index"
		echo ""

		while true
		do
		    read -p "Enter your choice: " start
		    case $start in
		    1 ) break
			;;
		    2 ) index
			;;
		    * ) echo "Invalid input!"
			;;
		    esac
		done

		# Input method
		clear
		echo "Where do you want to get input?"
		echo "-------------------------------"
		echo "1) Standard input stream"
		echo "2) File"
		echo ""
		while true
		do
		    read -p "Enter your choice: " start
		    case $start in
		    1 ) copy_type=1
		        break
			;;
		    2 ) copy_type=2
			break
			;;
		    * ) echo "Invalid input!"
			;;
		    esac
		done
	else
		# read file
		if [[ -z `find . -name "files.txt"` && -z `find . -name "directories.txt"` ]]; then
			echo "Error!!!"
			read -n 1 -s -r -p "Press any key to go back to index..."
			index
		fi

		# read to_path
		while true
		do
			read -p "Please enter the absolute path of the target directory:" to_path
	        if test -d $to_path
	        then
	            break
	        else
	            echo "Invalid input!"
	        fi
		done

		echo "You are to copy these items: "
		cat files.txt 2> /dev/null
		cat directories.txt 2> /dev/null

		while true
		do
			read -p "Continue? (y/n): " continue_delete
			case $continue_delete in
				y ) break
					;;
				n ) rm sorted.txt 2> /dev/null
					rm files.txt 2> /dev/null
					rm directories 2> /dev/null
					read -n 1 -s -r -p "Press any key to go back to index..."
					index
					;;
				* ) echo "Invalid input!"
					;;
			esac
		done

		old_IFS=$IFS
	    IFS='
	'
		if [[ $1 == f ]]; then
			for nameC in `cat files.txt`
	    	do
	        	copyFile $nameC $to_path
	    	done
	    else
	    	for nameC in `cat directories.txt`
	    	do
	        	copyDirectory $nameC $to_path
	    	done
		fi
	
	    IFS=$old_IFS
	    rm files.txt 2> /dev/null
	    rm directories.txt 2> /dev/null
	fi

	# standard copy procedure
	if test $copy_type -eq 1
	then
	    touch files.txt
	    touch directories.txt
	    while true
	    do
	        read -p "Please enter the absolute path of the file/directory, input '#' to stop:" nameC
	        if test -f "$nameC"
	        then
	            echo "$nameC" >> files.txt
	        elif test -d "$nameC"
	        then
	            echo "$nameC" >> directories.txt
	        elif [[ ! -e "$nameC" && "$nameC" != "#" ]]
	        then
	            echo "Invalid input!"
	        elif [ "$nameC" == "#" ]
	        then
	            break
	        fi
	    done
	    while true
	    do
	        read -p "Please enter the absolute path of the target directory:" to_path
	        if test -d "$to_path"
	        then
	            break
	        else
	            echo "Invalid input!"
	        fi
	    done
	    old_IFS=$IFS
	    IFS='
	'
	    for nameC in `cat files.txt`
	    do
	        copyFile $nameC $to_path
	    done
	    for nameC in `cat directories.txt`
	    do
	        copyDirectory $nameC $to_path
	    done
	    IFS=$old_IFS
	elif test $copy_type -eq 2
	then
	    
	    while true
	    do
	        read -p "Please enter the absolute path of the file that contains the path:" filenameC
	        read -p "Please enter the absolute path of the target directory:" to_path
	        if test -f $filenameC -a test -d $to_path
	        then
	            break
	        else
	            echo "Invalid input!"
	        fi
	    done
	    
	    old_IFS=$IFS
	    IFS='
	'
	    for nameC in `cat $filenameC`
	    do
	        if test -d $nameC
	        then
	            copyDirectory $nameC $to_path
	        elif test -f $nameC
	        then
	            copyFile $nameC $to_path
	        elif ! test -e $nameC
	        then
	            echo "$nameC doesn't exist!"
	        fi
	    done
	    IFS=$old_IFS
	fi

	rm files.txt 2> /dev/null
	rm directories.txt 2> /dev/null
}
############################# Copy program ends here ############################



############################# Delete program ends here ############################
function deleteFile()
{
    rm -f $1
    echo "$1 has been deleted!"
}

function deleteDirectory()
{
    rm -r -f $1
    echo "$1 has been deleted!"
}
function delete()
{
	if [[ -z $1 ]]; then
		# Welcome
		clear
		echo "Welcome!"
		echo "--------"
		echo "1) Start deleting"
		echo "2) Return to index"
		echo ""

		while true
		do
		    read -p "Enter your choice: " startD
		    case $startD in
		    1 ) break
			;;
		    2 ) rm sorted.txt 2> /dev/null
				rm files.txt 2> /dev/null
				rm directories 2> /dev/null
				index
			;;
		    * ) echo "Invalid input!"
			;;
		    esac
		done

		# Input method
		clear
		echo "Where do you want to get input?"
		echo "-------------------------------"
		echo "1) Standard input stream"
		echo "2) File"
		echo ""
		while true
		do
		    read -p "Enter your choice: " im
		    case $im in
		    1 ) delete_type=1
		        break
			;;
		    2 ) delete_type=2
			break
			;;
		    * ) echo "Invalid input!"
			;;
		    esac
		done
	else
		# read file
		if [[ -z `find . -name "files.txt"` && -z `find . -name "directories.txt"` ]]; then
			echo "Error!!!"
			read -n 1 -s -r -p "Press any key to go back to index..."
			index
		fi

		echo "You are to delete these items: "
		cat files.txt 2> /dev/null
		cat directories.txt 2> /dev/null

		while true
		do
			read -p "Continue? (y/n): " continue_delete
			case $continue_delete in
				y ) break
					;;
				n ) rm sorted.txt 2> /dev/null
					rm files.txt 2> /dev/null
					rm directories 2> /dev/null
					read -n 1 -s -r -p "Press any key to go back to index..."
					index
					;;
				* ) echo "Invalid input!"
					;;
			esac
		done

		# delete stuff
		old_IFS=$IFS
	    IFS='
	'
		case $1 in
			f ) for nameD in `cat files.txt`
	    		do
	        		deleteFile $nameD
	    		done
				;;
			d ) for nameD in `cat directories.txt`
			    do
			        deleteDirectory $nameD
			    done
		esac
	    
	    IFS=$old_IFS
	    rm files.txt 2> /dev/null
	    rm directories.txt 2> /dev/null

	    echo "Delete done!"
	    read -n 1 -s -r -p "Press any key to go back to index..."
		index
	fi
	
	# standard delete procedure
	if test $delete_type -eq 1
	then
		if [[ ! -z `find . -name "files.txt"` ]]; then
			rm files.txt
		fi

		if [[ ! -z `find . -name "directories.txt` ]]; then
			rm directories.txt
		fi

	    touch files.txt
	    touch directories.txt    
	    while true
	    do
	        read -p "Please enter the absolute path of the file/directory, input '#' to stop:" nameD
	        if test -f "$nameD"
	        then
	            echo "$nameD" >> files.txt
	        elif test -d "$nameD"
	        then 
	        	echo "$nameD" >> directories.txt
	        elif [[ ! -e "$nameD" && "$nameD" != "#" ]]
	        then
	            echo "Invalid input!"
	        elif test "$nameD" = "#"
	        then
	        	break
	        fi
	    done
	    old_IFS=$IFS
	    IFS='
	'
	    for nameD in `cat files.txt`
	    do
	        deleteFile $nameD
	    done

	    for nameD in `cat directories.txt`
	    do
	        deleteDirectory $nameD
	    done
	    IFS=$old_IFS

	elif test $delete_type -eq 2
	then
	    
	    while true
	    do
	        read -p "Please enter the absolute path of the file that contains the path:" filenameD
	        if test -f $filenameD
	        then
	            break
	        else
	            echo "Invalid input!"
	        fi
	    done
	    
	    old_IFS=$IFS
	    IFS='
	'
	    for nameD in `cat $filenameD`
	    do
	        if test -d $nameD
	        then
	            deleteDirectory $nameD
	        elif test -f $nameD
	        then
	            deleteFile $nameD
	        elif ! test -e $nameD
	        then
	            echo "$nameD doesn't exist!"
	        fi
	    done
	    IFS=$old_IFS
	fi

	rm files.txt 2> /dev/null
	rm directories.txt 2> /dev/null
}
############################# Delete program ends here ############################



################### Main program starts from here ###################
function index()
{
	if [[ ! -z $1 ]]; then
		finder_dir=`pwd`
	fi
	
	cd $finder_dir
	clear
	echo "This is Finder."
	echo "---------------"
	echo "1) Search"
	echo "2) Batch rename"
	echo "3) Search & batch rename files"
	echo "4) Copy"
	echo "5) Search & batch copy items"
	echo "6) Delete"
	echo "7) Search & batch delete items"
	echo "e) Exit"

	while true
	do
		read -p "Enter your choice: " go
		case $go in
			1 ) search
				;;
			2 ) rename
				;;
			3 ) search 3
				;;
			4 ) copy
				;;
			5 ) search 5
				;;
			6 ) delete
				;;
			7 ) search 7
				;;
			e ) exit 0
				;;
			* ) echo "Invalid input!"
				;;
		esac
	done
}

index 1
#################### Main program ends here ###########################

### update:
# 1. extension check
# 2. search engine can now filter out hidden files
# 3. better user logic