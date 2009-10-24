
#include "stdafx.h"
#include "HeeksCAD.h"
#include "AutoSave.h"
#include <sys/stat.h>
#include <wx/msgdlg.h>

CAutoSave::CAutoSave(const int interval, const bool skip_recovery /* = false */ )
{
	// We need to see if our backup file already exists.  If so, we should
	// prompt the user to see if they want to use its contents or ignore it.

	m_backup_file_name = _(".HeeksCAD_Backup_Data_File.heeks");

	m_save_interval = interval;	// Minutes

	struct stat statbuf;
	if ((stat(Ttc(m_backup_file_name.c_str()), &statbuf) != -1) && (! skip_recovery))
	{
		// The file was found.  If it still has size then we must have crashed.
		if (statbuf.st_size > 0)
		{
			int answer = wxMessageBox(_T("Do you want to recover using the automatic backup file?"),
						_T("Confirm"), wxYES_NO );
			if (answer == wxYES) 
			{
				printf("Recovering from backup file %s\n", Ttc(m_backup_file_name.c_str()));
				// wxGetApp().OpenXMLFile(m_backup_file_name.c_str());
				wxGetApp().OpenFile(m_backup_file_name.c_str(), false, NULL, false );
			}
		}
	}

	wxTimer::Start( m_save_interval * 60 * 1000, false );	// Save interval - minutes -> milliseconds
} // End default constructor

CAutoSave::~CAutoSave()
{
	// We need to delete the backup file on the assumption that the operator has
	// successfully saved or discarded their data in the 'normal way'.

	wxTimer::Stop();

	// Empty the file
	FILE *fp = fopen(Ttc(m_backup_file_name.c_str()),"w");
	if (fp) fclose(fp);

} // End destructor

/**
	This overloads the wxTimer::Notify() method.

	It gets called by the wxTimer class at regular intervals.
 */
void CAutoSave::Notify()
{
	wxGetApp().SaveXMLFile( m_backup_file_name.c_str() );

} // End Notify() method



